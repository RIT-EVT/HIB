#pragma once
#include <dev/ADS8689IPWR.hpp>
#include <dev/RedundantADC.hpp>
#include <sys/types.h>

namespace HIB {

constexpr size_t payloadLength = 8;

/**
 * HIB header file
 */
class HIB {
public:
    HIB(DEV::RedundantADC& throttle, DEV::RedundantADC& brake);

    void process();

    void readThrottleVoltage();

    void readBrakeVoltage();

    uint8_t payload[payloadLength];

private:
    DEV::RedundantADC& throttle;
    DEV::RedundantADC& brake;

};

}// namespace HIB