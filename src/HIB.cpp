#include "core/utils/log.hpp"
#include <HIB.hpp>
#include <dev/RedundantADC.hpp>

namespace io = core::io;

namespace HIB {

HIB::HIB(RedundantADC& throttle, RedundantADC& brake, HibPinMap pinMap, bool selfTest)
    : pinMap(pinMap), selfTest(selfTest), throttle(throttle), brake(brake) {}

// Reads the 2 sets of 3 ADCs and processes their errors while packaging them for CAN
io::CANMessage HIB::process() {
    // Check to make sure the bike is still supposed to be on
    if (!checkStartupPins()) {
        return io::CANMessage();
    }

    // Read in the voltages from the throttle
    readThrottleVoltage();
    // Brake is currently not implemented, but read the ADCs anyways
    readBrakeVoltage();

    if (throttleVoltage < VOLTAGE_DEADZONE) {
        throttleVoltage = 0;
    }

    if (brakeVoltage < VOLTAGE_DEADZONE) {
        brakeVoltage = 0;
    }

    hibPayload.throttleVoltageMSB = static_cast<uint8_t>(throttleVoltage >> 8 & 0xFF);
    hibPayload.throttleVoltageLSB = static_cast<uint8_t>(throttleVoltage & 0xFF);
    hibPayload.brakeVoltageMSB = static_cast<uint8_t>(brakeVoltage >> 8 & 0xFF);
    hibPayload.brakeVoltageLSB = static_cast<uint8_t>(brakeVoltage & 0xFF);

    if (acceptableThrottleMarginErrors > ACCEPTABLE_MARGIN_ERROR_COUNT) {
        acceptableThrottleMarginErrors = ACCEPTABLE_MARGIN_ERROR_COUNT + 1;
        hibPayload.throttleError = 1;
    }

    if (acceptableBrakeMarginErrors > ACCEPTABLE_MARGIN_ERROR_COUNT) {
        acceptableBrakeMarginErrors = ACCEPTABLE_MARGIN_ERROR_COUNT + 1;
        hibPayload.brakeError = 1;
    }

    if (precisionThrottleMarginErrors > PRECISION_MARGIN_ERROR_COUNT) {
        precisionThrottleMarginErrors = PRECISION_MARGIN_ERROR_COUNT + 1;
        hibPayload.throttleError = 2;
    }

    if (precisionBrakeMarginErrors > PRECISION_MARGIN_ERROR_COUNT) {
        precisionBrakeMarginErrors = PRECISION_MARGIN_ERROR_COUNT + 1;
        hibPayload.brakeError = 2;
    }

    if (comparisonThrottleErrors > COMPARISON_ERROR_COUNT) {
        comparisonThrottleErrors = COMPARISON_ERROR_COUNT + 1;
        hibPayload.throttleError = 3;
    }

    if (comparisonBrakeErrors > COMPARISON_ERROR_COUNT) {
        comparisonBrakeErrors = COMPARISON_ERROR_COUNT + 1;
        hibPayload.throttleError = 3;
    }

    uint8_t payload[payloadLength];
    payload[0] = hibPayload.throttleVoltageMSB;
    payload[1] = hibPayload.throttleVoltageLSB;
    payload[2] = hibPayload.brakeVoltageMSB;
    payload[3] = hibPayload.brakeVoltageLSB;
    payload[4] = hibPayload.throttleError;
    payload[5] = hibPayload.brakeError;

    return io::CANMessage{HIB_MESSAGE_ID, payloadLength, payload, false};
}

void HIB::readThrottleVoltage() {
    const RedundantADC::Status status = throttle.read(throttleVoltage);// gets the errors and voltage from the ADC cluster

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
    const RedundantADC::Status status = brake.read(brakeVoltage);// gets the errors and voltage from the ADC cluster

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

bool HIB::checkRunningPins() {
    bool isReady = true;
    if (pinMap.start->readPin() != io::GPIO::State::HIGH) {
        LOG_INFO("Error: The key is not turned to the ON position!\n\r");
        isReady = false;
    }
    if (pinMap.forwardEnable1->readPin() != io::GPIO::State::HIGH) {
        LOG_INFO("Error: The first of the forward enable pins is not on!\n\r");
        isReady = false;
    }
    if (pinMap.forwardEnable2->readPin() != io::GPIO::State::HIGH) {
        LOG_INFO("Error: The second of the forward pins is not on!\n\r");
        isReady = false;
    }
    if (pinMap.forwardEnable3->readPin() != io::GPIO::State::HIGH) {
        LOG_INFO("Error: The third of the forward enable pins is not on!\n\r");
        isReady = false;
    }
    if (pinMap.throttleSwitch->readPin() != io::GPIO::State::HIGH) {
        LOG_INFO("Error: The throttle switch is not on!\n\r");
        isReady = false;
    }
    return isReady;
}

}// namespace HIB