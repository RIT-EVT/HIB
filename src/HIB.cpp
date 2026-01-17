#include "core/io/CAN.hpp"
#include "core/io/types/CANMessage.hpp"
#include "core/utils/log.hpp"
#include <HIB.hpp>
#include <dev/RedundantADC.hpp>

namespace HIB {

HIB::HIB(RedundantADC& throttle, RedundantADC& brake)
    : throttle(throttle), brake(brake) {
    // Initialize payload to 0's
    for (uint8_t i = 0; i < payloadLength; i++) {
        payload[i] = 0;
    }
}

// Reads the 2 sets of 3 ADCs and processes their errors while packaging them for CAN
void HIB::process() {
    // Read in the voltages from the throttle
    readThrottleVoltage();
    // readBrakeVoltage();

    if (throttleVoltage < 475) {
        throttleVoltage = 0;
    }

    if (brakeVoltage < 450) {
        brakeVoltage = 0;
    }

    payload[0] = static_cast<uint8_t>(throttleVoltage >> 8 & 0xFF);
    payload[1] = static_cast<uint8_t>(throttleVoltage & 0xFF);
    payload[2] = static_cast<uint8_t>(brakeVoltage >> 8 & 0xFF);
    payload[3] = static_cast<uint8_t>(brakeVoltage & 0xFF);

    if (acceptableThrottleMarginErrors > ACCEPTABLE_MARGIN_ERROR_COUNT) {
        acceptableThrottleMarginErrors = ACCEPTABLE_MARGIN_ERROR_COUNT + 1;
        payload[4] |= 0b01;
    }

    if (acceptableBrakeMarginErrors > ACCEPTABLE_MARGIN_ERROR_COUNT) {
        acceptableBrakeMarginErrors = ACCEPTABLE_MARGIN_ERROR_COUNT + 1;
        payload[4] |= 0b10;
    }

    if (precisionThrottleMarginErrors > PRECISION_MARGIN_ERROR_COUNT) {
        precisionThrottleMarginErrors = PRECISION_MARGIN_ERROR_COUNT + 1;
        payload[4] |= 0b01;
    }

    if (precisionBrakeMarginErrors > PRECISION_MARGIN_ERROR_COUNT) {
        precisionBrakeMarginErrors = PRECISION_MARGIN_ERROR_COUNT + 1;
        payload[4] |= 0b10;
    }

    if (comparisonThrottleErrors > COMPARISON_ERROR_COUNT) {
        comparisonThrottleErrors = COMPARISON_ERROR_COUNT + 1;
        payload[4] |= 0b01;
    }

    if (comparisonBrakeErrors > COMPARISON_ERROR_COUNT) {
        comparisonBrakeErrors = COMPARISON_ERROR_COUNT + 1;
        payload[4] |= 0b10;
    }
}

void HIB::readThrottleVoltage() {
    const RedundantADC::Status status = throttle.read(throttleVoltage); // gets the errors and voltage from the ADC cluster

    // Increment the status of each error if it is received
    if (status == RedundantADC::Status::ACCEPTABLE_MARGIN_EXCEEDED) {
        acceptableThrottleMarginErrors++;
    }

    if (status == RedundantADC::Status::PRECISION_MARGIN_EXCEEDED) {
        precisionThrottleMarginErrors++;
    }

    if (status == RedundantADC::Status::COMPARISON_ERROR) {
        comparisonThrottleErrors++;
    }
}

void HIB::readBrakeVoltage() {
    const RedundantADC::Status status = brake.read(brakeVoltage); // gets the errors and voltage from the ADC cluster

    // Increment the status of each error if it is received
    if (status == RedundantADC::Status::ACCEPTABLE_MARGIN_EXCEEDED) {
        acceptableBrakeMarginErrors++;
    }

    if (status == RedundantADC::Status::PRECISION_MARGIN_EXCEEDED) {
        precisionBrakeMarginErrors++;
    }

    if (status == RedundantADC::Status::COMPARISON_ERROR) {
        comparisonBrakeErrors++;
    }
}

}