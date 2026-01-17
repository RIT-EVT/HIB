#ifndef HIB_
#define HIB_

#include <dev/RedundantADC.hpp>

using namespace std;

namespace HIB {

constexpr size_t payloadLength = 5;

/**
 * The Handlebar Interface Board Takes in a 0.0 to 12.0 volt signal from the throttle and brake
 * which it then converts to a 16 bit value through its "double-triple" ADC setup. It then sends
 * this voltage value along with any error codes that were generated to the Vehicle Control Unit
 * (VCU).
 *
 * It reads in the voltage from either the throttle or the brake through one set of three ADCs
 * that communicate with the microcontroller using SPI. After the controller sets up the devices
 * they are read in and then averaged and contrasted with the average to find any margin errors.
 *
 * The data after being correctly parsed and checked for errors is then sent through CAN to the
 * VCU for decision-making.
 */
class HIB {
public:
    HIB(RedundantADC& throttle, RedundantADC& brake);

    void process();

    uint8_t payload[payloadLength];
    uint16_t throttleVoltage = 0;
    uint16_t brakeVoltage = 0;

    // Counters for throttle errors
    uint64_t acceptableThrottleMarginErrors = 0;
    uint64_t precisionThrottleMarginErrors = 0;
    uint64_t comparisonThrottleErrors = 0;

    // Counters for brake errors
    uint64_t acceptableBrakeMarginErrors = 0;
    uint64_t precisionBrakeMarginErrors = 0;
    uint64_t comparisonBrakeErrors = 0;

private:
    void readThrottleVoltage();

    void readBrakeVoltage();

    RedundantADC& throttle;
    RedundantADC& brake;

    /**
     * Payload for CAN transmission.
     * Byte 0: MSB for Throttle Voltage
     * Byte 1: LSB for Throttle Voltage
     * Byte 2: MSB for Brake Voltage
     * Byte 3: LSB for Brake Voltage
     * Byte 4: Error Byte for triggering a shutdown
     * Byte 4 layout: X X X X X X X X
     *                7 6 5 4 3 2 1 0
     *                            ^ ^
     *                            | \
     *           ________________/   \__________________
     *           Brake Error Bit      Throttle Error Bit
     */
    uint8_t data[payloadLength];
};

}// namespace HIB
#endif