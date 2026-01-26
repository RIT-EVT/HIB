#include <core/utils/log.hpp>
#include <dev/ADS8689IPWR.hpp>

namespace HIB {
/**
 *
 * @param spi
 * @param deviceNumber
 */
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

// Oleg function (old read was narrowing uint32 -> uint8 or uint16)
uint16_t ADS8689IPWR::readVoltage() const {
    uint8_t bytes[4] = {0};
    spi.startTransmission(deviceNumber);
    spi.read(bytes, 4);
    spi.endTransmission(deviceNumber);

    uint16_t raw = (static_cast<uint16_t>(bytes[0]) << 8) | static_cast<uint16_t>(bytes[1]);

    uint32_t scaled = static_cast<uint32_t>(raw) * VOLTAGE_MAX / UINT16_MAX;

    return static_cast<uint16_t>(scaled);
}
}// namespace HIB
