#include <HIB.hpp>
#include <dev/RedundantADC.hpp>

namespace HIB {

HIB::HIB(DEV::RedundantADC& throttle, DEV::RedundantADC& brake)
    : throttle(throttle), brake(brake) {
    // Initialize payload to 0's
    for (uint8_t i = 0; i < payloadLength; i++) {
        payload[i] = 0;
    }
}

// Reads the 2 sets of 3 ADC's and processes their errors while packaging them for CAN
void HIB::process() {
    readThrottleVoltage();
    readBrakeVoltage();

    if (throttleVoltage < 300) {
        throttleVoltage = 0;
    }

    if (brakeVoltage < 300) {
        brakeVoltage = 0;
    }

    payload[0] = throttleVoltage >> 8;
    payload[1] = throttleVoltage & 0xFF;
    payload[2] = brakeVoltage >> 8;
    payload[3] = brakeVoltage & 0xFF;

    if (acceptableThrottleMarginErrors > ACCEPTABLE_MARGIN_ERROR_COUNT) {
        payload[4] = payload[4] | 0b00000001;
    }

    if (acceptableBrakeMarginErrors > ACCEPTABLE_MARGIN_ERROR_COUNT) {
        payload[4] = payload[4] | 0b00000010;
    }

    if (precisionThrottleMarginErrors > PRECISION_MARGIN_ERROR_COUNT) {
        payload[4] = payload[4] | 0b00000001;
    }

    if (precisionBrakeMarginErrors > PRECISION_MARGIN_ERROR_COUNT) {
        payload[4] = payload[4] | 0b00000010;
    }

    if (comparisonThrottleErrors > COMPARISON_ERROR_COUNT) {
        payload[4] = payload[4] | 0b00000001;
    }

    if (comparisonBrakeErrors > COMPARISON_ERROR_COUNT) {
        payload[4] = payload[4] | 0b00000010;
    }
}

void HIB::readThrottleVoltage() {
    const DEV::RedundantADC::Status status = throttle.read(throttleVoltage); // gets the errors and voltage from the ADC cluster

    // Increment the status of each error if it is received
    if (status == DEV::RedundantADC::Status::ACCEPTABLE_MARGIN_EXCEEDED) {
        acceptableThrottleMarginErrors++;
    }

    if (status == DEV::RedundantADC::Status::PRECISION_MARGIN_EXCEEDED) {
        precisionThrottleMarginErrors++;
    }

    if (status == DEV::RedundantADC::Status::COMPARISON_ERROR) {
        comparisonThrottleErrors++;
    }
}

void HIB::readBrakeVoltage() {
    DEV::RedundantADC::Status status = brake.read(brakeVoltage); // gets the errors and voltage from the ADC cluster

    // Increment the status of each error if it is received
    if (status == DEV::RedundantADC::Status::ACCEPTABLE_MARGIN_EXCEEDED) {
        acceptableBrakeMarginErrors++;
    }

    if (status == DEV::RedundantADC::Status::PRECISION_MARGIN_EXCEEDED) {
        precisionBrakeMarginErrors++;
    }

    if (status == DEV::RedundantADC::Status::COMPARISON_ERROR) {
        comparisonBrakeErrors++;
    }
}

}