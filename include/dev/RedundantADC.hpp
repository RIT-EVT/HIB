#ifndef REDUNDANT_ADC_
#define REDUNDANT_ADC_

#include <dev/ADS8689IPWR.hpp>

/**
 * Defined limits for the amount of times an error can occur before a restart is required
 */
#define COMPARISON_ERROR_COUNT 1
#define PRECISION_MARGIN_ERROR_COUNT 3
#define ACCEPTABLE_MARGIN_ERROR_COUNT 5

/**
 * CAN TX and RX pins
 */
#define CAN_TX io::Pin::PA_12
#define CAN_RX io::Pin::PA_11

namespace io = core::io;

namespace HIB {
/**
 * An object that takes in 3 different ADS8689IPWR objects to read out and compare their output
 * voltages. It compares and contrasts these voltages against their average to find the marginal
 * error for each of them. If the margin error is a little high, too high, or completely off in
 * one or more of the devices then it will return a corresponding error the HIB object.
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
    RedundantADC(ADS8689IPWR& adc0, ADS8689IPWR& adc1, ADS8689IPWR& adc2);

    /**
     * Read voltage readings from the ADCs and check for redundancy.
     *
     * This function reads values from three ADCs and checks for redundancy.
     *
     * @param[out] return_val Reference to the variable to store the value read from the ADCs
     * @return RedundantADC::Status The status of the processing.
     */
    Status read(uint16_t& return_val) const;

private:
    ADS8689IPWR& adc0;
    ADS8689IPWR& adc1;
    ADS8689IPWR& adc2;
};

}
#endif