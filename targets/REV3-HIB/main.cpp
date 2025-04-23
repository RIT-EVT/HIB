#include <HIB.hpp>
#include <chrono>
#include <core/io/GPIO.hpp>
#include <core/io/SPI.hpp>
#include <core/io/UART.hpp>
#include <core/manager.hpp>
#include <core/utils/time.hpp>
#include <dev/ADS8689IPWR.hpp>
#include <dev/RedundantADC.hpp>

namespace io   = core::io;

constexpr uint32_t SPI_SPEED = SPI_SPEED_500KHZ; // 500KHz
constexpr uint8_t deviceCount = 3;

#define ADS8689IPWR_RANGE_SEL_REG 0x14 // R/W Range selection register  ----

io::GPIO* throttleDevices[deviceCount];
io::GPIO* brakeDevices[deviceCount];

void canIRQHandler(io::CANMessage& message, void* priv) {
    io::UART* uart = (io::UART*) priv;
    uart->printf("Message received\r\n");
    uart->printf("Message id: 0x%X \r\n", message.getId());
    uart->printf("Message length: %d\r\n", message.getDataLength());
    uart->printf("Message contents: ");

    uint8_t* message_payload = message.getPayload();
    for (int i = 0; i < message.getDataLength(); i++) {
        uart->printf("0x%02X ", message_payload[i]);
    }
    uart->printf("\r\n\r\n");
}

int main() {
    // Initialize system
    core::platform::init();

    // Initialize UART
    io::UART& uart = io::getUART<io::Pin::UART_TX, io::Pin::UART_RX>(9600);

    // Initialize CAN
    io::CAN& can = io::getCAN<io::Pin::PA_12, io::Pin::PA_11>(true);

    // Set up each device
    brakeDevices[0] = &io::getGPIO<io::Pin::PB_9>(io::GPIO::Direction::OUTPUT);
    brakeDevices[0]->writePin(io::GPIO::State::HIGH);

    brakeDevices[1] = &io::getGPIO<io::Pin::PB_8>(io::GPIO::Direction::OUTPUT);
    brakeDevices[1]->writePin(io::GPIO::State::HIGH);

    brakeDevices[2] = &io::getGPIO<io::Pin::PB_7>(io::GPIO::Direction::OUTPUT);
    brakeDevices[2]->writePin(io::GPIO::State::HIGH);

    throttleDevices[0] = &io::getGPIO<io::Pin::PC_7>(io::GPIO::Direction::OUTPUT);
    throttleDevices[0]->writePin(io::GPIO::State::HIGH);

    throttleDevices[1] = &io::getGPIO<io::Pin::PC_8>(io::GPIO::Direction::OUTPUT);
    throttleDevices[1]->writePin(io::GPIO::State::HIGH);

    throttleDevices[2] = &io::getGPIO<io::Pin::PC_9>(io::GPIO::Direction::OUTPUT);
    throttleDevices[2]->writePin(io::GPIO::State::HIGH);

    // Create the two SPI clusters
    io::SPI& spiThrottle = io::getSPI<io::Pin::PB_10, io::Pin::PB_15, io::Pin::PB_14>(throttleDevices, deviceCount);
    io::SPI& spiBrake = io::getSPI<io::Pin::PC_10, io::Pin::PC_12, io::Pin::PC_11>(brakeDevices, deviceCount);

    // Configure the systems
    spiThrottle.configureSPI(SPI_SPEED, io::SPI::SPIMode::SPI_MODE0, SPI_MSB_FIRST);
    spiBrake.configureSPI(SPI_SPEED, io::SPI::SPIMode::SPI_MODE0, SPI_MSB_FIRST);

    // Create each ADS8689IPWR object
    auto throttleADC1 = HIB::DEV::ADS8689IPWR(spiThrottle, 0);
    auto throttleADC2 = HIB::DEV::ADS8689IPWR(spiThrottle, 1);
    auto throttleADC3 = HIB::DEV::ADS8689IPWR(spiThrottle, 2);
    auto brakeADC1 = HIB::DEV::ADS8689IPWR(spiBrake, 0);
    auto brakeADC2 = HIB::DEV::ADS8689IPWR(spiBrake, 1);
    auto brakeADC3 = HIB::DEV::ADS8689IPWR(spiBrake, 2);

    // Create the Redundant ADC's
    auto throttle = HIB::DEV::RedundantADC(throttleADC1, throttleADC2, throttleADC3);
    auto brake = HIB::DEV::RedundantADC(brakeADC1, brakeADC2, brakeADC3);

    // Finally create the HIB object to begin processing data
    auto hib = HIB::HIB(throttle, brake);

    // ID for HIB is 0x0D0
    io::CANMessage transmit_message(0x0D0, 5, &hib.payload[0], false);
    io::CANMessage received_message;

    // Try to join the network
    io::CAN::CANStatus result = can.connect();
    can.enableEmergencyFilter(ENABLE);
    // Begin CAN Tests
    uart.printf("Starting CAN testing\r\n");

    if (result !=io::CAN::CANStatus::OK) {
        uart.printf("Failed to connect to CAN network\r\n");
        return 1;
    }

    // main
    int count = 0;

    // Speed Loop (Comment out to get display
    while (true) {
        hib.process();
        io::CANMessage transmit_message(0x0D0, 5, &hib.payload[0], false);
        can.receive(&received_message, false);
    }

    // Display Loop
    while (true) {
        // Process the voltage
        hib.process();

        // ID for HIB is 0x0D0
        io::CANMessage transmit_message(0x0D0, 5, &hib.payload[0], false);

        // Try to send the message
        result = can.transmit(transmit_message);
        if (result !=io::CAN::CANStatus::OK) {
            uart.printf("Failed to transmit message\r\n");
            return 1;
        }

        // Try to receive the message
        result = can.receive(&received_message, false);
        if (result != io::CAN::CANStatus::OK) {
            uart.printf("Failed to receive message\r\n");
            continue;
        }

        // Check if data was received
        if (received_message.getDataLength() == 0) {
            uart.printf("Message filtered out!");
        }  else {
            uart.printf("Message received\r\n");
            uart.printf("Message id: %d \r\n", received_message.getId());
            uart.printf("Message length: %d\r\n", received_message.getDataLength());
            uart.printf("Message contents: ");

            const uint8_t* message_payload = received_message.getPayload();
            for (int i = 0; i < received_message.getDataLength(); i++) {
                uart.printf("0x%02X ", message_payload[i]);
            }

            uart.printf("\r\n");

            for (int i = 0; i < received_message.getDataLength(); i++) {
                uart.printf("0x%02X ", hib.payload[i]);
            }

            // CAN most likely make variables to store total errors over the entire runtime to get a better picture
            uart.printf("\r\n");
            uart.printf("Throttle Voltage: %i mV\r\n", message_payload[0] << 8 | message_payload[1]);
            uart.printf("Brake Voltage: %i mV\r\n", message_payload[2] << 8 | message_payload[3]);
            uart.printf("Error Code (1 = Throttle, 2 = Brake, 3 = Both): %i\r\n", message_payload[4]);
            uart.printf("Throttle Voltage: %i mV\r\n", hib.throttleVoltage);
            uart.printf("Brake Voltage: %i mV\r\n", hib.brakeVoltage);
            uart.printf("Throttle Acceptable Errors: %i\r\n", hib.acceptableThrottleMarginErrors);
            uart.printf("Throttle Precision Errors: %i\r\n", hib.precisionThrottleMarginErrors);
            uart.printf("Throttle Comparison Errors: %i\r\n", hib.comparisonThrottleErrors);
            uart.printf("Brake Acceptable Errors: %i\r\n", hib.acceptableBrakeMarginErrors);
            uart.printf("Brake Precision Errors: %i\r\n", hib.precisionBrakeMarginErrors);
            uart.printf("Brake Comparison Errors: %i\r\n", hib.comparisonBrakeErrors);
            count++;
            core::time::wait(1000);
        }
        uart.printf("\r\n\r\n");
    }

    return 0;
}