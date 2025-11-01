#pragma once
#include <dev/RedundantADC.hpp>
#include <sys/types.h>

namespace HIB {

/**
 * Counts that when exceeded throw an error to the VC
 */
#define COMPARISON_ERROR_COUNT 1
#define PRECISION_MARGIN_ERROR_COUNT 3
#define ACCEPTABLE_MARGIN_ERROR_COUNT 5

constexpr size_t payloadLength = 5;

/**
 * HIB header file
 */
class HIB {
public:
    HIB(DEV::RedundantADC& throttle, DEV::RedundantADC& brake);

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

    DEV::RedundantADC& throttle;
    DEV::RedundantADC& brake;

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