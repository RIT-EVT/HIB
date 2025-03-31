#pragma once
#include <core/io/GPIO.hpp>
#include <core/manager.hpp>
#include <core/io/SPI.hpp>
#include <core/io/UART.hpp>
#include <core/utils/time.hpp>

namespace IO = core::io;

namespace ADS8689IPWR {
class ADS8689IPWR {
public:
    // Initiates the setup process and prepares the component for voltage readings
    ADS8689IPWR(IO::SPI& spi);

    // Reads the voltage signal from the ADC
    uint32_t read();

    // Writes to a register on the chip
    // Meant to be used during the setup process
    uint32_t write(uint32_t data);
private:
    // Reference to the SPI object that is communicating with the chip
    IO::SPI& spi;
};
}
