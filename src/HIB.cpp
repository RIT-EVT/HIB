#include <HIB.hpp>


namespace HIB {
    HIB::HIB(const DEV::RedundantADC& throttle, const DEV::RedundantADC& brake)
        : throttlePosition(0), brakePosition(0), noErrors(0), precisionErrors(0), marginErrors(0),
        comparisonErrors(0), throttle(throttle), brake(brake) {}

    // "Run" the HIB and update the payload
    void HIB::process() {
        getPositions();
        makePayload();
    }

    uint16_t HIB::getVoltage(DEV::RedundantADC& red_adc) {
        uint32_t voltage = 0; // Voltage to be recieved in millivolts
        DEV::RedundantADC::Status status = red_adc.readVoltage(voltage); // gets the errors and voltage from the ADC cluster
        if (status == DEV::RedundantADC::Status::OK) {
            noErrors++;
        } else if (status == DEV::RedundantADC::Status::PRECISION_MARGIN_EXCEEDED) {
            precisionErrors++;
        } else if (status == DEV::RedundantADC::Status::ACCEPTABLE_MARGIN_EXCEEDED) {
            marginErrors++;
        } else if (status == DEV::RedundantADC::Status::COMPARISON_ERROR) {
            comparisonErrors++;
        } return voltage;
    }

    // Get the position voltage from throttle and break
    void HIB::getPositions() {
        throttlePosition = getVoltage(throttle);
        brakePosition = getVoltage(brake);
    }

    // Form payload
    void HIB::makePayload() {
        payload[0] = throttlePosition >> 8; // Bitshift to get the most sig
        payload[1] = throttlePosition & 0xFF; // Erases the most significant bits to ensure data is only collectd from the first 8 bits
        payload[2] = brakePosition >> 8; // Same as above
        payload[3] = brakePosition & 0xFF; // Same as above
        payload[4] = noErrors;
        payload[5] = precisionErrors;
        payload[6] = marginErrors;
        payload[7] = comparisonErrors;
    }

}// namespace HIB