/**
 * RedundantADC test target. Run this on the board to test the functionality of the RedundantADC
 * class and the physical setup of the two redundant adc setups on thd board
 */
#include <core/io/ADC.hpp>
#include <HIB.hpp>
#include <core/io/GPIO.hpp>
#include <core/manager.hpp>
#include <dev/RedundantADC.hpp>
#include <core/utils/log.hpp>

namespace io = core::io;
namespace time = core::time;

constexpr uint8_t deviceCount = 3;

io::GPIO* throttleDevices[deviceCount];
io::GPIO* brakeDevices[deviceCount];

namespace HIB {
int main() {
    // Initialize system
    core::platform::init();

    // Initialize UART and logger
    io::UART& uart = io::getUART<io::Pin::UART_TX, io::Pin::UART_RX>(9600);
    core::log::LOGGER.setUART(&uart);
    core::log::LOGGER.setLogLevel(core::log::Logger::LogLevel::INFO);

    // Set up each chip select pin
    brakeDevices[0] = &io::getGPIO<BRAKE_0>(io::GPIO::Direction::OUTPUT);
    brakeDevices[0]->writePin(io::GPIO::State::HIGH);
    brakeDevices[1] = &io::getGPIO<BRAKE_1>(io::GPIO::Direction::OUTPUT);
    brakeDevices[1]->writePin(io::GPIO::State::HIGH);
    brakeDevices[2] = &io::getGPIO<BRAKE_2>(io::GPIO::Direction::OUTPUT);
    brakeDevices[2]->writePin(io::GPIO::State::HIGH);

    throttleDevices[0] = &io::getGPIO<THROTTLE_0>(io::GPIO::Direction::OUTPUT);
    throttleDevices[0]->writePin(io::GPIO::State::HIGH);
    throttleDevices[1] = &io::getGPIO<THROTTLE_1>(io::GPIO::Direction::OUTPUT);
    throttleDevices[1]->writePin(io::GPIO::State::HIGH);
    throttleDevices[2] = &io::getGPIO<THROTTLE_2>(io::GPIO::Direction::OUTPUT);
    throttleDevices[2]->writePin(io::GPIO::State::HIGH);

    // Create the two SPI buses and configure them
    io::SPI& spiThrottle = io::getSPI<THROTTLE_SPI_SCK, THROTTLE_SPI_MOSI, THROTTLE_SPI_MISO>(throttleDevices, deviceCount);
    spiThrottle.configureSPI(SPI_SPEED, SPI_MODE, SPI_MSB_FIRST);
    io::SPI& spiBrake = io::getSPI<BRAKE_SPI_SCK, BRAKE_SPI_MOSI, BRAKE_SPI_MISO>(brakeDevices, deviceCount);
    spiBrake.configureSPI(SPI_SPEED, SPI_MODE, SPI_MSB_FIRST);

    // Create all 6 ADS8689IPWR objects
    auto throttleAdc1 = ADS8689IPWR(spiThrottle, 0);
    auto throttleAdc2 = ADS8689IPWR(spiThrottle, 1);
    auto throttleAdc3 = ADS8689IPWR(spiThrottle, 2);
    auto brakeAdc1 = ADS8689IPWR(spiBrake, 0);
    auto brakeAdc2 = ADS8689IPWR(spiBrake, 1);
    auto brakeAdc3 = ADS8689IPWR(spiBrake, 2);

    // Initialize the two redundant ADCs
    auto throttle = RedundantADC(throttleAdc1, throttleAdc2, throttleAdc3);
    auto brake = RedundantADC(brakeAdc1, brakeAdc2, brakeAdc3);

    // Declare voltage variables
    uint16_t throttleVoltage = 0;
    uint16_t brakeVoltage = 0;

    while (true) {
        // Process ADC values
        RedundantADC::Status throttleStatus = throttle.read(throttleVoltage);
        RedundantADC::Status brakeStatus = brake.read(brakeVoltage);

        //check ADC Statuses
        if (throttleStatus == RedundantADC::Status::OK) {
            LOG_INFO("Throttle Average Voltage Reading: %dV\r\n", throttleVoltage);
        } else if (throttleStatus == RedundantADC::Status::PRECISION_MARGIN_EXCEEDED) {
            LOG_INFO("Throttle Precision error detected\r\n");
        } else if (throttleStatus == RedundantADC::Status::ACCEPTABLE_MARGIN_EXCEEDED) {
            LOG_INFO("Throttle Margin error detected\r\n");
        } else if (throttleStatus == RedundantADC::Status::COMPARISON_ERROR) {
            LOG_INFO("Throttle Comparison error detected\r\n");
        }

        //check ADC Statuses
        if (brakeStatus == RedundantADC::Status::OK) {
            LOG_INFO("Brake Average Voltage Reading: %dV\r\n", brakeVoltage);
        } else if (brakeStatus == RedundantADC::Status::PRECISION_MARGIN_EXCEEDED) {
            LOG_INFO("Brake Precision error detected\r\n");
        } else if (brakeStatus == RedundantADC::Status::ACCEPTABLE_MARGIN_EXCEEDED) {
            LOG_INFO("Brake Margin error detected\r\n");
        } else if (brakeStatus == RedundantADC::Status::COMPARISON_ERROR) {
            LOG_INFO("Brake Comparison error detected\r\n\r\n");
        }

        time::wait(1000);
    }
}
}
