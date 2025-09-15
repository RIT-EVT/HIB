#include <HIB.hpp>
#include <chrono>
#include <core/io/GPIO.hpp>
#include <core/io/SPI.hpp>
#include <core/io/UART.hpp>
#include <core/manager.hpp>
#include <core/utils/time.hpp>
#include <dev/ADS8689IPWR.hpp>
#include <dev/RedundantADC.hpp>
#include <core/utils/log.hpp>

namespace io = core::io;
namespace devh = HIB::DEV;
namespace hib = HIB;

constexpr uint32_t SPI_SPEED = SPI_SPEED_500KHZ; // 500KHz
constexpr uint8_t deviceCount = 3;

#define ADS8689IPWR_RANGE_SEL_REG 0x14 // R/W Range selection register  ----

io::GPIO* throttleDevices[deviceCount];
io::GPIO* brakeDevices[deviceCount];

void canIRQHandler(io::CANMessage& message, void* priv) {
    io::UART* uart = (io::UART*) priv;
    core::log::LOGGER.log(core::log::Logger::LogLevel::INFO, "Message received\r\n");
    core::log::LOGGER.log(core::log::Logger::LogLevel::INFO, "Message id: 0x%X \r\n", message.getId());
    core::log::LOGGER.log(core::log::Logger::LogLevel::INFO, "Message length: %d\r\n", message.getDataLength());
    core::log::LOGGER.log(core::log::Logger::LogLevel::INFO, "Message contents: ");

    uint8_t* message_payload = message.getPayload();
    for (int i = 0; i < message.getDataLength(); i++) {
        core::log::LOGGER.log(core::log::Logger::LogLevel::INFO,"0x%02X ", message_payload[i]);
    }
    core::log::LOGGER.log(core::log::Logger::LogLevel::INFO,"\r\n\r\n");
}

int main() {
    // Count for the number of iterations of the process
    uint64_t count = 0;

    // Initialize system
    core::platform::init();

    // Initialize UART
    io::UART& uart = io::getUART<io::Pin::UART_TX, io::Pin::UART_RX>(9600);

    core::log::LOGGER.setUART(&uart);
    core::log::LOGGER.setLogLevel(core::log::Logger::LogLevel::INFO);

    // Initialize CAN
    io::CAN& can = io::getCAN<io::Pin::PA_12, io::Pin::PA_11>(true);

    // Brake ADC setup
    brakeDevices[0] = &io::getGPIO<io::Pin::PB_9>(io::GPIO::Direction::OUTPUT);
    brakeDevices[0]->writePin(io::GPIO::State::HIGH);
    brakeDevices[1] = &io::getGPIO<io::Pin::PB_8>(io::GPIO::Direction::OUTPUT);
    brakeDevices[1]->writePin(io::GPIO::State::HIGH);
    brakeDevices[2] = &io::getGPIO<io::Pin::PB_7>(io::GPIO::Direction::OUTPUT);
    brakeDevices[2]->writePin(io::GPIO::State::HIGH);

    io::SPI& spiBrake = io::getSPI<io::Pin::PC_10, io::Pin::PC_12, io::Pin::PC_11>(brakeDevices, deviceCount);
    spiBrake.configureSPI(SPI_SPEED, io::SPI::SPIMode::SPI_MODE0, SPI_MSB_FIRST);

    devh::ADS8689IPWR brakeADC1 = devh::ADS8689IPWR(spiBrake, 0);
    devh::ADS8689IPWR brakeADC2 = devh::ADS8689IPWR(spiBrake, 1);
    devh::ADS8689IPWR brakeADC3 = devh::ADS8689IPWR(spiBrake, 2);
    devh::RedundantADC brake = devh::RedundantADC(brakeADC1, brakeADC2, brakeADC3);

    // Throttle ADC setup
    throttleDevices[0] = &io::getGPIO<io::Pin::PC_7>(io::GPIO::Direction::OUTPUT);
    throttleDevices[0]->writePin(io::GPIO::State::HIGH);
    throttleDevices[1] = &io::getGPIO<io::Pin::PC_8>(io::GPIO::Direction::OUTPUT);
    throttleDevices[1]->writePin(io::GPIO::State::HIGH);
    throttleDevices[2] = &io::getGPIO<io::Pin::PC_9>(io::GPIO::Direction::OUTPUT);
    throttleDevices[2]->writePin(io::GPIO::State::HIGH);

    io::SPI& spiThrottle = io::getSPI<io::Pin::PB_10, io::Pin::PB_15, io::Pin::PB_14>(throttleDevices, deviceCount);
    spiThrottle.configureSPI(SPI_SPEED, io::SPI::SPIMode::SPI_MODE0, SPI_MSB_FIRST);

    devh::ADS8689IPWR throttleADC1 = devh::ADS8689IPWR(spiThrottle, 0);
    devh::ADS8689IPWR throttleADC2 = devh::ADS8689IPWR(spiThrottle, 1);
    devh::ADS8689IPWR throttleADC3 = devh::ADS8689IPWR(spiThrottle, 2);
    devh::RedundantADC throttle = devh::RedundantADC(throttleADC1, throttleADC2, throttleADC3);

    // Finally create the HIB object to begin processing data
    hib::HIB hib = hib::HIB(throttle, brake);

    // Try to join the network
    io::CAN::CANStatus result = can.connect();
    can.enableEmergencyFilter(ENABLE);

    // Begin CAN Tests
    core::log::LOGGER.log(core::log::Logger::LogLevel::INFO,"Starting CAN testing\r\n");

    if (result !=io::CAN::CANStatus::OK) {
        core::log::LOGGER.log(core::log::Logger::LogLevel::INFO,"Failed to connect to CAN network\r\n");
        return 1;
    }

    // Main loop
    while (true) {
        // Process the voltage
        hib.process();

        // ID for HIB is 0x0D0
        io::CANMessage transmit_message(0x0D0, 5, &hib.payload[0], false);
        io::CANMessage received_message;

        // Try to send the message
        result = can.transmit(transmit_message);
        if (result !=io::CAN::CANStatus::OK) {
            core::log::LOGGER.log(core::log::Logger::LogLevel::INFO,"Failed to transmit message\r\n");
            return 1;
        }

        // Try to receive the message
        result = can.receive(&received_message, false);
        if (result != io::CAN::CANStatus::OK) {
            core::log::LOGGER.log(core::log::Logger::LogLevel::INFO,"Failed to receive message\r\n");
            continue;
        }

        // Give the state of the payload
        // Count is counting the number of loops
        if (count >= 2000) {
            // uart.printf("\033[2J\033[H");
            // Check if data was received
            if (received_message.getDataLength() == 0) {
                core::log::LOGGER.log(core::log::Logger::LogLevel::INFO,"Message filtered out!");
            }
            else {
                const uint8_t* message_payload = received_message.getPayload();
                core::log::LOGGER.log(core::log::Logger::LogLevel::INFO, "CAN payload: \033[32m");

                for (int i = 0; i < received_message.getDataLength(); i++) {
                    core::log::LOGGER.log(core::log::Logger::LogLevel::INFO,"0x%02X ", message_payload[i]);
                }

                core::log::LOGGER.log(core::log::Logger::LogLevel::INFO,"\r\n\033[37mHIB payload: \033[34m");

                for (int i = 0; i < received_message.getDataLength(); i++) {
                    core::log::LOGGER.log(core::log::Logger::LogLevel::INFO,"0x%02X ", hib.payload[i]);
                }

                core::log::LOGGER.log(core::log::Logger::LogLevel::INFO,"\r\n\033[37m");
                core::log::LOGGER.log(core::log::Logger::LogLevel::INFO,"Throttle Voltage: %i mV\r\n", (message_payload[0] << 8 | message_payload[1]));
                core::log::LOGGER.log(core::log::Logger::LogLevel::INFO,"Error Code: %i\r\n", message_payload[4]);
            }

            count = 0;
        }

        count++;
    }

    return 0;
}