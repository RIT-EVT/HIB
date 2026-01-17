/**
 * REV3-HIB main target.
 */

#include <HIB.hpp>
#include <dev/RedundantADC.hpp>
#include <dev/ADS8689IPWR.hpp>
#include <core/manager.hpp>
#include <core/utils/log.hpp>
#include <core/io/GPIO.hpp>
#include <core/io/SPI.hpp>
#include <core/io/UART.hpp>

namespace io = core::io;

constexpr uint8_t deviceCount = 3;

io::GPIO* throttleDevices[deviceCount];
io::GPIO* brakeDevices[deviceCount];

namespace HIB {
void canIRQHandler(io::CANMessage& message, void* priv) {
    core::log::LOGGER.log(
        core::log::Logger::LogLevel::INFO,
        "Message received\r\n"
        "Message id: 0x%X \r\n"
        "Message length: %d\r\n"
        "Message contents: ",
        message.getId(),
        message.getDataLength()
    );

    uint8_t* message_payload = message.getPayload();
    for (int i = 0; i < message.getDataLength(); i++) {
        core::log::LOGGER.log(core::log::Logger::LogLevel::INFO,"0x%02X ", message_payload[i]);
    }
    core::log::LOGGER.log(core::log::Logger::LogLevel::INFO,"\r\n\r\n");
}

int main() {
    // Initialize system
    core::platform::init();

    // Initialize UART
    io::UART& uart = io::getUART<io::Pin::UART_TX, io::Pin::UART_RX>(9600);

    core::log::LOGGER.setUART(&uart);
    core::log::LOGGER.setLogLevel(core::log::Logger::LogLevel::INFO);

    // Initialize CAN
    io::CAN& can = io::getCAN<io::Pin::PA_12, io::Pin::PA_11>(true);

    // Brake ADC setup
    brakeDevices[0] = &io::getGPIO<BRAKE_0>(io::GPIO::Direction::OUTPUT);
    brakeDevices[0]->writePin(io::GPIO::State::HIGH);
    brakeDevices[1] = &io::getGPIO<BRAKE_1>(io::GPIO::Direction::OUTPUT);
    brakeDevices[1]->writePin(io::GPIO::State::HIGH);
    brakeDevices[2] = &io::getGPIO<BRAKE_2>(io::GPIO::Direction::OUTPUT);
    brakeDevices[2]->writePin(io::GPIO::State::HIGH);

    // Initialize the brake SPI array
    io::SPI& spiBrake = io::getSPI<BRAKE_SPI_SCK, BRAKE_SPI_MOSI, BRAKE_SPI_MISO>(brakeDevices, deviceCount);
    spiBrake.configureSPI(SPI_SPEED, SPI_MODE, SPI_MSB_FIRST);

    // Initialize each brake ADC and then initialize the redundant ADC
    ADS8689IPWR brakeADC1 = ADS8689IPWR(spiBrake, 0);
    ADS8689IPWR brakeADC2 = ADS8689IPWR(spiBrake, 1);
    ADS8689IPWR brakeADC3 = ADS8689IPWR(spiBrake, 2);
    RedundantADC brake = RedundantADC(brakeADC1, brakeADC2, brakeADC3);

    throttleDevices[0] = &io::getGPIO<THROTTLE_0>(io::GPIO::Direction::OUTPUT);
    throttleDevices[0]->writePin(io::GPIO::State::HIGH);
    throttleDevices[1] = &io::getGPIO<THROTTLE_1>(io::GPIO::Direction::OUTPUT);
    throttleDevices[1]->writePin(io::GPIO::State::HIGH);
    throttleDevices[2] = &io::getGPIO<THROTTLE_2>(io::GPIO::Direction::OUTPUT);
    throttleDevices[2]->writePin(io::GPIO::State::HIGH);

    // Initialize the throttle SPI array
    io::SPI& spiThrottle = io::getSPI<THROTTLE_SPI_SCK, THROTTLE_SPI_MOSI, THROTTLE_SPI_MISO>(throttleDevices, deviceCount);
    spiThrottle.configureSPI(SPI_SPEED, SPI_MODE, SPI_MSB_FIRST);

    // Initialize each throttle ADC and then initialize the redundant ADC
    ADS8689IPWR throttleADC1 = ADS8689IPWR(spiThrottle, 0);
    ADS8689IPWR throttleADC2 = ADS8689IPWR(spiThrottle, 1);
    ADS8689IPWR throttleADC3 = ADS8689IPWR(spiThrottle, 2);
    RedundantADC throttle = RedundantADC(throttleADC1, throttleADC2, throttleADC3);

    // Finally create the HIB object to begin processing data
    HIB hib = HIB(throttle, brake);

    // Try to join the network
    io::CAN::CANStatus result = can.connect();

    // ID for HIB is 0x0D0
    io::CANMessage transmit_message(0x0D0, 5, hib.payload, false);

    // Try to send the message
    result = can.transmit(transmit_message);
    if (result != io::CAN::CANStatus::OK) {
        LOG_INFO("Failed to transmit message\r\n");
    }

    // Begin CAN Test
    LOG_INFO("Starting CAN testing\r\n");

    // Main loop
    while (true) {
        // Process the voltage
        hib.process();
        transmit_message = io::CANMessage(0x0D0, 5, hib.payload, false);

        // Try to send the message
        result = can.transmit(transmit_message);
        if (result != io::CAN::CANStatus::OK) {
            LOG_INFO("Failed to transmit message\r\n");
        }
    }

    return 0;
}
}