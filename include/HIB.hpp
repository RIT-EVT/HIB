#pragma once

#include <cstdint>
#include <dev/RedundantADC.hpp>

namespace HIB {

/**
 * HIB header file in Branch
 */
class HIB {
private:
    uint16_t throttlePosition;
    uint16_t brakePosition;
    uint8_t noErrors, precisionErrors, marginErrors, comparisonErrors;
    DEV::RedundantADC throttle, brake;
public:
    /// Payload for the CAN transmission
    uint8_t payload[8];

    /// Gets the RedundantADCs for the throttle and break voltages
    HIB(const DEV::RedundantADC& throttle, const DEV::RedundantADC& brake);

    /// Runs through all the necessary function to update the payload
    void process();

    /// Creates a payload for the CAN bus to transmit.
    void makePayload();

    /// Reads the throttles position and returns it in millivolts
    uint16_t getVoltage(DEV::RedundantADC& adc);

    /// Set the positions of the throttle and brake
    void getPositions();
};

}// namespace HIB