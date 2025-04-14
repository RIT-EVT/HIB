#include <dev/ADS8689IPWR.hpp>

namespace HIB::DEV {


ADS8689IPWR::ADS8689IPWR(io::SPI& spi, const uint8_t deviceNumber) : spi(spi), deviceNumber(deviceNumber) {
    // Pull CS low
    spi.startTransmission(deviceNumber);
    spi.write(0b11010000);
    spi.write(RANGE_SEL_REG);
    spi.write(0b00000000);
    spi.write(TWELVE_VOLT_SCALER);
    // Pull CS high
    spi.endTransmission(deviceNumber);

    // begin the conversion
    spi.startTransmission(deviceNumber);
    // NOP command
    spi.write(0b0);
    spi.write(0b0);
    spi.write(0b0);
    spi.write(0b0);
    spi.endTransmission(deviceNumber);
}

uint16_t ADS8689IPWR::read() {
    uint8_t bytes[4];
    uint16_t voltage;

    spi.startTransmission(deviceNumber);
    // NOP command
    spi.write(0b0);
    spi.write(0b0);
    spi.write(0b0);
    spi.write(0b0);
    spi.read(bytes, 4);
    spi.endTransmission(deviceNumber);

    voltage = bytes[2] << 8 | bytes[3];
    voltage *= voltage * 12288 / 65536;
    return voltage;
}

}
