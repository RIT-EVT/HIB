#include <HIB.hpp>
#include <dev/RedundantADC.hpp>
#include <core/manager.hpp>
#include <core/io/CAN.hpp>
#include <core/io/ADC.hpp>
#include <core/io/pin.hpp>

namespace IO = core::io;

int main() {
    core::platform::init();

    // Initialize devices
    IO::CAN& can = IO::getCAN<IO::Pin::PA_12, IO::Pin::PA_11>(true);
    IO::UART& uart = IO::getUART<IO::Pin::UART_TX, IO::Pin::UART_RX>(9600);
    IO::ADC& adc0 = IO::getADC<IO::Pin::PC_0>();
    IO::ADC& adc1 = IO::getADC<IO::Pin::PA_5>();
    IO::ADC& adc2 = IO::getADC<IO::Pin::PA_6>();

    // Initialize HIB
    const HIB::DEV::RedundantADC throttle(adc0, adc1, adc2);
    HIB::HIB hib(throttle);

    // ID for HIB is 0x0D0
    IO::CANMessage transmit_message(0x0D0, 8, hib.getPayload(), false);
    IO::CANMessage received_message;

    // Try to join the network
    IO::CAN::CANStatus result = can.connect();

    //  can.addCANFilter(0, 0, 13);  //This would create a filter that allows all messages through
    can.addCANFilter(0xD0, 0xFF0, 0);
    can.enableEmergencyFilter(ENABLE);

    // Begin CAN Tests
    uart.printf("Starting CAN testing\r\n");

    if (result != IO::CAN::CANStatus::OK) {
        uart.printf("Failed to connect to CAN network\r\n");
        return 1;
    }

    // MAIN loop
    while (true) {
        // Process the voltage
        hib.process();

        // Try to send the message
        result = can.transmit(transmit_message);
        if (result != IO::CAN::CANStatus::OK) {
            uart.printf("Failed to transmit message\r\n");
            return 1;
        }

        // Try to recieve the message
        result = can.receive(&received_message, false);
        if (result != IO::CAN::CANStatus::OK) {
            uart.printf("Failed to receive message\r\n");
            continue;
        }

        // Check if data was recieved
        if (received_message.getDataLength() == 0) {
            uart.printf("Message filtered out!");
        } else {
            uart.printf("Message received\r\n");
            uart.printf("Message id: %d \r\n", received_message.getId());
            uart.printf("Message length: %d\r\n", received_message.getDataLength());
            uart.printf("Message contents: ");

            uint8_t* message_payload = received_message.getPayload();
            for (int i = 0; i < received_message.getDataLength(); i++) {
                uart.printf("0x%02X ", message_payload[i]);
            }

            // CAN most likely make variables to store total errors over the entire runtime to get a better picture
            uart.printf("\r\n");
            uart.printf("Throttle Voltage: %i mV\r\n", message_payload[0] << 8 | message_payload[1]);
            uart.printf("Brake Voltage: %i mV\r\n", message_payload[2] << 8 | message_payload[3]);
            uart.printf("No Errors: %i\r\n", message_payload[4]);
            uart.printf("Precision Errors: %i\r\n", message_payload[5]);
            uart.printf("Margin Errors: %i\r\n", message_payload[6]);
            uart.printf("Comparison Errors: %i\r\n", message_payload[7]);
        }
        uart.printf("\r\n\r\n");

        core::time::wait(2000);
    }

    return 0;
}