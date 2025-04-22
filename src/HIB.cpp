#include <HIB.hpp>
#include <dev/RedundantADC.hpp>

namespace HIB {

HIB::HIB(DEV::RedundantADC& throttle, DEV::RedundantADC& brake)
    : throttle(throttle), brake(brake) {
    // Initialize payload to 0's
    for (int i = 0; i < payloadLength; i++) {
        payload[i] = 0;
    }
}

void HIB::process() {
    HIB::readThrottleVoltage();
    HIB::readBrakeVoltage();
}

void HIB::readThrottleVoltage() {
    uint32_t voltage = 0; // Voltage to be received in millivolts
    DEV::RedundantADC::Status status = throttle.read(voltage); // gets the errors and voltage from the ADC cluster

    payload[0] += voltage;
    payload[1] += voltage >> 8;

    // Increment the status of each
    if (status == DEV::RedundantADC::Status::OK) {
        payload[4]++;
    }
}

void HIB::readBrakeVoltage() {
    uint32_t voltage = 0; // Voltage to be recieved in millivolts
    DEV::RedundantADC::Status status = brake.read(voltage); // gets the errors and voltage from the ADC cluster

    payload[2] += voltage;
    payload[3] += voltage >> 8;
}

}