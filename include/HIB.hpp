#pragma once
#include <dev/RedundantADC.hpp>
#include <core/manager.hpp>
#include <cstdint>

namespace IO = core::io;

namespace HIB {

class HIB {
public:
    /**
     * Constructor for the HIB
     * Takes a redundant ADC reference
     * @param throttle: const RedundantADC reference
     */
    HIB(const DEV::RedundantADC& throttle);

    /**
     * Main function for the HIB. Runs all subroutines for things like reading voltage
     */
    void process();

    /**
     * Takes an adc to read from and the payload index of the most significant byte for the voltage
     * from the redundant adc
     * @param adc: The redundant adc that the voltage should be read from
     * @param volts: The payload index that the most significant
     * byte of voltage should be stored at
     */
    void setVoltage(DEV::RedundantADC adc, uint8_t* volts);

   /**
    * Gets the payload for reading from
    * @return: The address of the first index of the payload
    */
    uint8_t* getPayload();

private:
    /** 8 byte payload stores data in this order:
     * [0] - Most Significant Throttle Byte
     * [1] - Least Significant Throttle Byte
     * [2] - Most Significant Blank Byte for Brake
     * [3] - Least Significant Blank Byte for Brake
     * [4] - # of OK's
     * [5] - # of PRECISION_MARGIN_EXCEEDED errors
     * [6] - # of ACCEPTABLE_MARGIN_EXCEEDED errors
     * [7] - # of COMPARISON_ERROR errors
     */
    uint8_t payload[8];
    size_t payloadLength = 8;

    /// The triple adc setup attached to the throttle
    DEV::RedundantADC throttle;
};

}
