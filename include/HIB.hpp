#pragma once
#include <dev/RedundantADC.hpp>
#include <cstdint>

namespace IO = core::io;

namespace HIB {
class HIB {
public:
/** 8 byte payload stores data in this order:
 * [0] - Most Significant Throttle Byte
 * [1] - Least Significant Throttle Byte
 * [2] - Most Significant Blank Byte for Brake
 * [3] - Least Significant Blank Byte for Brake
 * [4] - # of OK's
 * [5] - # of PRECISION_MARGIN_EXCEEDED errors
 * [6] - # of ACCEPTABLE_MARGIN_EXCEEDED errors
 * [7] - # of COMPARISON_ERROR errors
 */
uint8_t payload[8];
const uint8_t payloadLength = 8;

/**
 * Constructor for the HIB
 * Takes a redundant ADC reference
 * @param throttle: RedundantADC reference
 */
HIB(DEV::RedundantADC& throttle);

/**
 * Main function for the HIB. Runs all subroutines for things like reading voltage
 */
void process();

/**
 * Reads in the voltage from the throttle redundant adc and increments the errors acquired
 */
void readThrottleVoltage();

/**
 * Reads in the voltage from the brake redundant adc and increments the errors acquired
 */
void readBrakeVoltage();

private:
/**
 * The triple adc setup attached to the throttle
 */
DEV::RedundantADC throttle;
};
}
