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
    HIB::readThrottleVoltage();
    HIB::readBrakeVoltage();

    if (acceptableThrottleMarginErrors > ACCEPTABLE_MARGIN_ERROR_COUNT) {
        payload[4] = payload[4] | 0b00000001;
    }

    if (acceptableBrakeMarginErrors > ACCEPTABLE_MARGIN_ERROR_COUNT) {
        payload[4] = payload[4] | 0b00000010;
    }

    if (precisionThrottleMarginErrors < PRECISION_MARGIN_ERROR_COUNT) {
        payload[4] = payload[4] | 0b00000001;
    }

    if (precisionBrakeMarginErrors > PRECISION_MARGIN_ERROR_COUNT) {
        payload[4] = payload[4] | 0b00000010;
    }

    if (comparisonThrottleErrors < COMPARISON_ERROR_COUNT) {
        payload[4] = payload[4] | 0b00000001;
    }

    if (comparisonBrakeErrors < COMPARISON_ERROR_COUNT) {
        payload[4] = payload[4] | 0b00000010;
    }
}

void HIB::readThrottleVoltage() {
    uint32_t voltage = 0; // Voltage to be received in millivolts
    const DEV::RedundantADC::Status status = throttle.read(voltage); // gets the errors and voltage from the ADC cluster
    payload[0] += voltage >> 8;
    payload[1] += voltage & 0xff;

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
    uint32_t voltage = 0; // Voltage to be received in millivolts
    DEV::RedundantADC::Status status = brake.read(voltage); // gets the errors and voltage from the ADC cluster

    payload[2] += voltage >> 8;
    payload[3] += voltage & 0xff;

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