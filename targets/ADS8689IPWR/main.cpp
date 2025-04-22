#include <HIB.hpp>
#include <core/dev/LED.hpp>
#include <core/io/GPIO.hpp>
#include <core/io/SPI.hpp>
#include <core/io/UART.hpp>
#include <core/manager.hpp>
#include <core/utils/log.hpp>
#include <core/utils/time.hpp>
#include <dev/ADS8689IPWR.hpp>
#include <dev/RedundantADC.hpp>

namespace io   = core::io;
namespace time = core::time;

constexpr uint32_t SPI_SPEED = SPI_SPEED_500KHZ; // 500KHz
constexpr uint8_t deviceCount = 3;

io::GPIO* throttleDevices[deviceCount];
io::GPIO* brakeDevices[deviceCount];

int main() {
    // Initialize system
    core::platform::init();

    // Initialize UART
    io::UART& uart = io::getUART<io::Pin::UART_TX, io::Pin::UART_RX>(9600);
    core::log::LOGGER.setUART(&uart);
    core::log::LOGGER.setLogLevel(core::log::Logger::LogLevel::INFO);

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

    // main loop
    while (true) {
        hib.process();
        uart.printf("Throttle Voltage: %i mV\r\n", hib.payload[0] << 8 | hib.payload[1]);
        uart.printf("Brake Voltage: %i mV\r\n", hib.payload[2] << 8 | hib.payload[3]);
        uart.printf("OK: %i\r\n", hib.payload[4]);
        uart.printf("PRECISION_MARGIN_EXCEEDED: %i\r\n", hib.payload[5]);
        uart.printf("ACCEPTABLE_MARGIN_EXCEEDED: %i\r\n", hib.payload[6]);
        uart.printf("COMPARISON_ERROR: %i\r\n", hib.payload[7]);
        time::wait(1000);
    }
}