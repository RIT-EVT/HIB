#pragma once

#include <core/io/GPIO.hpp>
#include <core/io/SPI.hpp>

#define RANGE_SEL_REG 0x14 // R/W Range selection register  ----
#define TWELVE_VOLT_SCALER 0b1000
#define HALF_WORD_WRITE 0b11010000
#define EMPTY_BYTE 0b00000000
#define NOP {EMPTY_BYTE, EMPTY_BYTE, EMPTY_BYTE, EMPTY_BYTE}
#define UINT16_MAX 65535
#define VOLTAGE_MAX 12288

namespace io   = core::io;

namespace HIB::DEV {

class ADS8689IPWR {
public:
    /**
     * Creates a ADS8689IPWR object while configuring the range selector.
     *
     * @param spi SPI reference to interface with the ADC
     * @param deviceNumber The number of the cs pin that the device is
     */
    ADS8689IPWR(io::SPI& spi, uint8_t deviceNumber);

    /**
     * Reads out the voltage from the ADC with a blank command through SPI
     *
     * @param voltage reference to variable that is storing voltage
     * @return the status of whether the transaction was successful
     */
    uint16_t read();
private:
    io::SPI& spi;
    const uint8_t deviceNumber;
};

}// namespace HIB::DEV
