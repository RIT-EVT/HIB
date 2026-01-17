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

constexpr uint8_t deviceCount = 3;

io::GPIO* throttleDevices[deviceCount];
io::GPIO* brakeDevices[deviceCount];

namespace HIB {
int main() {
    // Initialize system
    core::platform::init();

    // Setup UART
    io::UART& uart = io::getUART<io::Pin::UART_TX, io::Pin::UART_RX>(9600);
    core::log::LOGGER.setUART(&uart);
    core::log::LOGGER.setLogLevel(core::log::Logger::LogLevel::INFO);

    // Brake ADC setup
    brakeDevices[0] = &io::getGPIO<BRAKE_0>(io::GPIO::Direction::OUTPUT);
    brakeDevices[0]->writePin(io::GPIO::State::HIGH);
    brakeDevices[1] = &io::getGPIO<:BRAKE_1>(io::GPIO::Direction::OUTPUT);
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

    // Variables to store ADC values
    uint32_t return_val;

    while (1) {
        // Process ADC values
        RedundantADC::Status status = redundantADC.readVoltage(return_val);

        //check ADC Statuses0
        if (status == RedundantADC::Status::OK) {
            uart.printf("Average Voltage Reading %d\r\n", return_val);
        } else if (status == RedundantADC::Status::PRECISION_MARGIN_EXCEEDED) {
            uart.printf("One error detected\r\n");
        } else if (status == RedundantADC::Status::ACCEPTABLE_MARGIN_EXCEEDED) {
            uart.printf("Margin error detected\r\n");
        } else if (status == RedundantADC::Status::COMPARISON_ERROR) {
            uart.printf("Comparison error detected\r\n");
        }
    }
}
}
