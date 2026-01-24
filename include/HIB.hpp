#ifndef HIB_
#define HIB_

#include <dev/RedundantADC.hpp>

namespace HIB {
#define VCU_CAN_ID 0xD0

constexpr size_t payloadLength = 6;

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

    /**
     * Reads the voltages and errors from the redundant ADCs and returns a can message containing
     * the data from each read
     * @return The can message that should be transmitted
     */
    io::CANMessage process();

    /**
     * Payload for CAN transmission.
     * Byte 0: MSB for Throttle Voltage
     * Byte 1: LSB for Throttle Voltage
     * Byte 2: MSB for Brake Voltage
     * Byte 3: LSB for Brake Voltage
     * Byte 4: Error Byte for Throttle
     * Byte 5: Error Byte for Brake
     * Error Byte Layout: 0: No Error, 1: Precision Error, 2: Margin Error, 3: Comparison Error
     */
    struct {
        uint8_t throttleVoltageMSB = 0;
        uint8_t throttleVoltageLSB = 0;
        uint8_t brakeVoltageMSB = 0;
        uint8_t brakeVoltageLSB = 0;
        uint8_t throttleError = 0;
        uint8_t brakeError = 0;
    } hibPayload;
private:
    void readThrottleVoltage();

    void readBrakeVoltage();

    RedundantADC& throttle;
    RedundantADC& brake;

    uint16_t throttleVoltage = 0;
    uint16_t brakeVoltage = 0;

    // Counters for throttle errors
    uint32_t acceptableThrottleMarginErrors = 0;
    uint32_t precisionThrottleMarginErrors = 0;
    uint32_t comparisonThrottleErrors = 0;

    // Counters for brake errors
    uint32_t acceptableBrakeMarginErrors = 0;
    uint32_t precisionBrakeMarginErrors = 0;
    uint32_t comparisonBrakeErrors = 0;
};
}
#endif