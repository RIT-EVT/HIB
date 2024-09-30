#pragma once

#include <cstdint>

namespace HIB {

/**
 * HIB header file in Branch
 */
class HIB {
private:
  uint16_t throttlePosition;
  uint16_t brakePosition;
  uint8_t offByOneErrors;
  uint8_t marginErrors;
  uint8_t comparisonErrors;
  uint8_t thresholdInformation;
public:
  /// Creates a payload for the CAN bus to transmit.
  uint8_t* makePayload();
  uint16_t getThrottlePosition();
  uint16_t getBrakePosition();
};

}// namespace HIB