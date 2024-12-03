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
    uint8_t noErrors;
    uint8_t precisionErrors;
    uint8_t marginErrors;
    uint8_t comparisonErrors;
    DEV::RedundantADC throttle;
    DEV::RedundantADC brake;
    uint8_t payload[8];
public:
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

    /// Read the voltage from the triple ADC setup
    uint16_t readV(uint16_t& adc1, uint16_t& adc2, uint16_t& adc3);
};

}// namespace HIB