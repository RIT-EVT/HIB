#include <core/manager.hpp>
#include <core/io/pin.hpp>
#include <core/io/UART.hpp>
#include <core/io/GPIO.hpp>
#include <core/utils/time.hpp>

namespace io   = core::io;
namespace time = core::time;

constexpr uint32_t SPI_SPEED = SPI_SPEED_2MHZ; // 2MHz
constexpr uint8_t deviceCount = 6;

io::GPIO* devices[deviceCount];

int main() {
    // Initialize system
    core::platform::init();

    // ------ Begin DAC and Self-Test pulldowns ------
    // DAC1 pulldown
    io::GPIO& dac1 = io::getGPIO<io::Pin::PA_4>();
    dac1.writePin(io::GPIO::State::LOW);

    // DAC2 pulldown
    io::GPIO& dac2 = io::getGPIO<io::Pin::PA_5>();
    dac2.writePin(io::GPIO::State::LOW);

    // Self-test1 pulldown
    io::GPIO& self_test1 = io::getGPIO<io::Pin::PA_6>();
    self_test1.writePin(io::GPIO::State::LOW);

    // Self-test2 pulldown
    io::GPIO& self_test2 = io::getGPIO<io::Pin::PA_7>();
    self_test2.writePin(io::GPIO::State::LOW);
    // ------ End DAC and Self-Test pulldowns ------

    devices[0] = &io::getGPIO<io::Pin::SPI_CS>(io::GPIO::Direction::OUTPUT);
    devices[0]->writePin(io::GPIO::State::HIGH);

    io::SPI& spi = io::getSPI<io::Pin::SPI_SCK, io::Pin::SPI_MOSI, io::Pin::SPI_MISO>(devices, deviceCount);

    spi.configureSPI(SPI_SPEED, io::SPI::SPIMode::SPI_MODE3, SPI_MSB_FIRST);

    io::UART& uart = io::getUART<io::Pin::UART_TX, io::Pin::UART_RX>(9600);
}