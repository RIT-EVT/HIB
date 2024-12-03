#include <HIB.hpp>
#include <dev/RedundantADC.hpp>
#include <cstdint>

namespace HIB {

HIB::HIB(const DEV::RedundantADC& throttle)
    : throttle(throttle) {
    // Clear payload
    for (int i = 0; i < payloadLength; i++) {
        payload[i] = 0;
    }
}

void HIB::process() {
    HIB::setVoltage(throttle, &payload[0]);
}

void HIB::setVoltage(DEV::RedundantADC adc, uint8_t* volts) {
    uint32_t voltage = 0; // Voltage to be recieved in millivolts
    DEV::RedundantADC::Status status = adc.readVoltage(voltage); // gets the errors and voltage from the ADC cluster
    if (status == DEV::RedundantADC::Status::OK) {
        payload[4]++;
    } else if (status == DEV::RedundantADC::Status::PRECISION_MARGIN_EXCEEDED) {
        payload[5]++;
    } else if (status == DEV::RedundantADC::Status::ACCEPTABLE_MARGIN_EXCEEDED) {
        payload[6]++;
    } else if (status == DEV::RedundantADC::Status::COMPARISON_ERROR) {
        payload[7]++;
    }

    // Set the given byte pointer and the index after to the voltage given
    // Bit-masking just in case
    volts[0] = (voltage >> 24) & 0xFF;
    volts[1] = (voltage >> 16) & 0xFF;
}

uint8_t* HIB::getPayload() {
    return &payload[0];
}

}