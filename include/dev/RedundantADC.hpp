#pragma once

#include <EVT/io/ADC.hpp>
#include <cstdint>

namespace IO = EVT::core::IO;

namespace RedundantADC {

/**
 * This class allows processing readings from multiple ADCs and checking for redundancy.
 */
class RedundantADC {
public:
    /**
     * @brief Enum class representing the status of RedundantADC processing.
     */
    enum class Status {
        OK = 0,                  /**< No error. */
        OFF_BY_ONE_ERROR = 1,    /**< The reading is off by 1*/
        MARGIN_ERROR = 2,        /**< The reading is off by less than the specific margin*/
        COMPARISON_ERROR = 3,    /**< The readings do not match.*/
        CAN_LOOPBACK_ERROR = 4   /**< CAN loopback data does not match.*/
    };

    /**
     * @brief Constructs a RedundantADC object with the given ADC instances.
     *
     * @param[in] adc0 The first ADC instance.
     * @param[in] adc1 The second ADC instance.
     * @param[in] adc2 The third ADC instance.
     */
    RedundantADC(IO::ADC& adc0, IO::ADC& adc1, IO::ADC& adc2);

    /**
     * @brief Process readings from the ADCs and check for redundancy.
     *
     * This function reads values from three ADCs and checks for redundancy.
     *
     * @param[in] val1 Reference to the variable to store the value read from the first ADC.
     * @param[in] val2 Reference to the variable to store the value read from the second ADC.
     * @param[in] val3 Reference to the variable to store the value read from the third ADC.
     * @return RedundantADC::Status The status of the processing.
     */
    RedundantADC::Status process(uint32_t& val1, uint32_t& val2, uint32_t& val3);

private:
    IO::ADC& adc0; /**< Reference to the first ADC. */
    IO::ADC& adc1; /**< Reference to the second ADC. */
    IO::ADC& adc2; /**< Reference to the third ADC. */
};

}// namespace RedundantADC
