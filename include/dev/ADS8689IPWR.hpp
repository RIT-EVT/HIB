#ifndef ADS8689IPWR_
#define ADS8689IPWR_

#include <core/io/SPI.hpp>

/**
 * Read/Write Range selection register for changing the range of the ADC to 0V-12V
 */
#define RANGE_SEL_REG 0x14

/**
 * The 2 byte value used to change the range selection register from 3V3 to 12V0 when writing to it
 */
#define TWELVE_VOLT_SCALER 0b1000

/**
 * Used for writing to the range selection register to alter the range read in by the ADC
 */
#define HALF_WORD_WRITE 0b11010000

/**
 * Empty byte defined for use in the No-Operation command
 */
#define EMPTY_BYTE 0b00000000

/**
 * Defined No-Operation command
 */
#define NOP \
    { EMPTY_BYTE, EMPTY_BYTE, EMPTY_BYTE, EMPTY_BYTE }

/**
 * The maximum voltage that can be read in by the ADC | Used for transforming the given 2 bytes
 * from the ADC into a usable value that will be sent to the Vehicle Control Unit
 */
#define VOLTAGE_MAX 12288

/**
 * EVT info logger macro
 * @param text The formatted text that should be logged out
 */
#define LOG_INFO(text, ...) core::log::LOGGER.log(core::log::Logger::LogLevel::INFO, text, ##__VA_ARGS__)

/**
 * Brake Chip Select Pins
 */
#define BRAKE_0 io::Pin::PB_9
#define BRAKE_1 io::Pin::PB_8
#define BRAKE_2 io::Pin::PB_7

/**
 * Throttle Chip Select Pins
 * */
#define THROTTLE_0 io::Pin::PC_9
#define THROTTLE_1 io::Pin::PC_8
#define THROTTLE_2 io::Pin::PC_7

/**
 * The exact speed that the ADCs need to run at
 */
#define SPI_SPEED SPI_SPEED_500KHZ

/**
 * The mode that spi should run in
 */
#define SPI_MODE io::SPI::SPIMode::SPI_MODE0

#define BRAKE_SPI_SCK io::Pin::PC_10
#define BRAKE_SPI_MOSI io::Pin::PC_12
#define BRAKE_SPI_MISO io::Pin::PC_11

#define THROTTLE_SPI_SCK io::Pin::PB_10
#define THROTTLE_SPI_MOSI io::Pin::PB_15
#define THROTTLE_SPI_MISO io::Pin::PB_14

namespace io = core::io;

namespace HIB {
/**
 * Device driver for the ADS8689IPWR 16-bit ADC. Contains a constructor that runs the necessary
 * setup for the device and a read function that reads in the voltage from the ADC and transforms
 * it into a useful number (the actual numerical millivoltage)
 */
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
     * @return the status of whether the transaction was successful
     */
    uint16_t readVoltage() const;

private:
    io::SPI& spi;
    const uint8_t deviceNumber;
};
}// namespace HIB
#endif
