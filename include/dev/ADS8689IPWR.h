#pragma once
#include <core/io/GPIO.hpp>
#include <core/manager.hpp>
#include <core/io/SPI.hpp>
#include <core/io/UART.hpp>
#include <core/utils/time.hpp>

namespace io = core::io;

namespace HIB::DEV {
class ADS8689IPWR {
public:
    ADS8689IPWR(io::SPI& spi);
private:
};
}
