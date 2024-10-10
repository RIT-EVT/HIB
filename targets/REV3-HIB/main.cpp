/**
 * This is a basic sample of using the UART module. The program provides a
 * basic echo functionality where the uart will write back whatever the user
 * enters.
 */

#include <EVT/io/UART.hpp>
#include <EVT/io/pin.hpp>
#include <EVT/manager.hpp>
#include <HIB.hpp>
#include <dev/RedundantADC.hpp>

namespace IO = EVT::core::IO;

int main() {
    // Initialize system
    EVT::core::platform::init();

    // Get CAN instance with loopback enabled
    IO::CAN& can = IO::getCAN<IO::Pin::PA_12, IO::Pin::PA_11>(true);
    IO::UART& uart = IO::getUART<IO::Pin::UART_TX, IO::Pin::UART_RX>(9600);

    // TODO: Change these to be the correct pins for the throttle and brake ADCs
    IO::ADC& adc0 = IO::getADC<IO::Pin::PA_0>();
    IO::ADC& adc1 = IO::getADC<IO::Pin::PA_1>();
    IO::ADC& adc2 = IO::getADC<IO::Pin::PA_4>();

    // Create RedundantADC object
    const HIB::DEV::RedundantADC throttleADC(adc0, adc1, adc2);

    IO::ADC& adc3 = IO::getADC<IO::Pin::PA_3>();
    IO::ADC& adc4 = IO::getADC<IO::Pin::PA_5>();
    IO::ADC& adc5 = IO::getADC<IO::Pin::PA_6>();

    // Create RedundantADC object
    const HIB::DEV::RedundantADC brakeADC(adc3, adc4, adc5);
    // TODO: End of necessary ADC changes

    HIB::HIB hib = HIB::HIB(throttleADC, brakeADC);

    // ID is 0x0D0
    IO::CANMessage transmit_message(0x0D0, 8, &hib.payload[0], false);
    IO::CANMessage received_message;

    // Try to join the network
    IO::CAN::CANStatus result = can.connect();

    //  can.addCANFilter(0, 0, 13);  //This would create a filter that allows all messages through
    can.addCANFilter(0x0D0, 0b0000111111110000, 0);
    can.enableEmergencyFilter(ENABLE);

    uart.printf("Starting CAN testing\r\n");

    if (result != IO::CAN::CANStatus::OK) {
        uart.printf("Failed to connect to CAN network\r\n");
        return 1;
    }

    // MAIN loop
    while (true) {
        hib.process();

        result = can.transmit(transmit_message);
        if (result != IO::CAN::CANStatus::OK) {
            uart.printf("Failed to transmit message\r\n");
            return 1;
        }

        result = can.receive(&received_message, false);
        if (result != IO::CAN::CANStatus::OK) {
            uart.printf("Failed to receive message\r\n");
            continue;
        }

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

        EVT::core::time::wait(2999);
    }

    return 0;
}
