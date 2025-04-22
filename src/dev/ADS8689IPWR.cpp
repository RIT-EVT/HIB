#include "../../libs/EVT-core/samples/canopen/canopen_sample/TestCanNode.hpp"

#include <core/utils/log.hpp>
#include <dev/ADS8689IPWR.hpp>

namespace HIB::DEV {

ADS8689IPWR::ADS8689IPWR(io::SPI& spi, const uint8_t deviceNumber) : spi(spi), deviceNumber(deviceNumber) {
    uint8_t message[4] = {HALF_WORD_WRITE, RANGE_SEL_REG, EMPTY_BYTE, TWELVE_VOLT_SCALER};
    spi.startTransmission(deviceNumber);
    spi.write(message, 4);
    spi.endTransmission(deviceNumber);

    // Begin the conversion with a NOP command
    uint8_t nop[4] = NOP;
    spi.startTransmission(deviceNumber);
    spi.write(nop, 4);
    spi.endTransmission(deviceNumber);
}

uint16_t ADS8689IPWR::read() {
    uint8_t bytes[4];
    spi.startTransmission(deviceNumber);
    spi.read(bytes, 4);
    spi.endTransmission(deviceNumber);

    // First byte is MSB
    uint32_t voltage = bytes[0] << 8 | bytes[1];
    // Normalize the received info (divide by uint16_t(MAX)) and scale accordingly
    voltage = voltage / UINT16_MAX * VOLTAGE_MAX;
    core::log::LOGGER.log(core::log::Logger::LogLevel::INFO, "byte0 = %x", bytes[0]);
    core::log::LOGGER.log(core::log::Logger::LogLevel::INFO, "byte1 = %x", bytes[1]);;
    core::log::LOGGER.log(core::log::Logger::LogLevel::INFO, "voltage = %u", voltage);
    return voltage;
}

}
