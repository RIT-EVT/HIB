#pragma once

#include <EVT/io/ADC.hpp>

namespace IO = EVT::core::IO;

namespace HIB::DEV {

/**
 * This class allows processing readings from redundant ADCs and checking for errors
 */

class RedundantADC {
public:
    /**
     * Enum class representing the status of RedundantADC processing.
     */
    enum Status {
        /** No error */
        OK = 0,
        /** The reading is off by more than precision margin but less than the accepted margin */
        PRECISION_MARGIN_EXCEEDED = 1,
        /** The reading is off by less than the acceptable margin */
        ACCEPTABLE_MARGIN_EXCEEDED = 2,
        /** The readings do not match */
        COMPARISON_ERROR = 3,
    };

    /**
     * Constructs a RedundantADC object with the given ADC instances.
     *
     * @param[in] adc0 The first ADC instance.
     * @param[in] adc1 The second ADC instance.
     * @param[in] adc2 The third ADC instance.
     */
    RedundantADC(IO::ADC& adc0, IO::ADC& adc1, IO::ADC& adc2);

    /**
     * Read voltage readings from the ADCs and check for redundancy.
     *
     * This function reads values from three ADCs and checks for redundancy.
     *
     * @param[out] return_val Reference to the variable to store the value read from the ADCs
     * @param[in] val2 Reference to the variable to store the value read from the second ADC.
     * @param[in] val3 Reference to the variable to store the value read from the third ADC.
     * @return RedundantADC::Status The status of the processing.
     */
    RedundantADC::Status readVoltage(uint32_t& return_val);

private:
    /** Reference to the first ADC. */
    IO::ADC& adc0;
    /** Reference to the second ADC. */
    IO::ADC& adc1;
    /** Reference to the third ADC. */
    IO::ADC& adc2;
};

}// namespace HIB::DEV
