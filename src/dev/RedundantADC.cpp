#include <EVT/io/ADC.hpp>
#include <dev/RedundantADC.hpp>

namespace IO = EVT::core::IO;

constexpr uint32_t LOW_MARGIN = 1;
constexpr uint32_t HIGH_MARGIN = 5;

namespace RedundantADC {

/*
 * Constructs a RedundantADC object with the given ADC instances.
 *
 * @param[in] adc0 The first ADC instance.
 * @param[in] adc1 The second ADC instance.
 * @param[in] adc2 The third ADC instance.
 */
RedundantADC::RedundantADC(IO::ADC& adc0, IO::ADC& adc1, IO::ADC& adc2) : adc0(adc0), adc1(adc1), adc2(adc2) {}

/*
 * Process readings from the ADCs and check for redundancy.
 *
 * This function reads values from three ADCs, performs bitwise comparison,
 * and checks for redundancy among the readings.
 *
 * @param[out] val1 Reference to the variable to store the value read from the first ADC.
 * @param[out] val2 Reference to the variable to store the value read from the second ADC.
 * @param[out] val3 Reference to the variable to store the value read from the third ADC.
 * @return RedundantADC::Status The status of the processing.
 */
RedundantADC::Status RedundantADC::readVoltage(uint32_t& return_val) {
    // Read ADC values
    int32_t adcValues[3];
    adcValues[0] = static_cast<int32_t>(adc0.read() * 1000);
    adcValues[1] = static_cast<int32_t>(adc1.read() * 1000);
    adcValues[2] = static_cast<int32_t>(adc2.read() * 1000);

    // Calculate average of all ADC values
    uint32_t average = (adcValues[0] + adcValues[1] + adcValues[2]) / 3;

    // Check for margin error
    bool adc0underLow = (adcValues[0] / average) < LOW_MARGIN;
    bool adc1underLow = (adcValues[1] / average) < LOW_MARGIN;
    bool adc2underLow = (adcValues[2] / average) < LOW_MARGIN;

    bool adc0underHigh = (adcValues[0] / average) < HIGH_MARGIN;
    bool adc1underHigh = (adcValues[1] / average) < HIGH_MARGIN;
    bool adc2underHigh = (adcValues[2] / average) < HIGH_MARGIN;

    // Check for redundancy
    bool allUnderLow = adc0underLow && adc1underLow && adc2underLow;
    bool allUnderHigh = adc0underHigh && adc1underHigh && adc2underHigh;

    if (allUnderLow) {
        return_val = average;
        return RedundantADC::Status::OK;
    } else if (allUnderHigh) {
        return_val = 0;
        return RedundantADC::Status::COMPARISON_ERROR;
    }

    // Off by one error check
    if (adc0underHigh && adc1underLow && adc2underLow) {
        return_val = (adcValues[1] + adcValues[2]) / 2;
        return RedundantADC::Status::OFF_BY_ONE_ERROR;
    } else if (adc0underLow && adc1underHigh && adc2underLow) {
        return_val = (adcValues[0] + adcValues[2]) / 2;
        return RedundantADC::Status::OFF_BY_ONE_ERROR;
    } else if (adc0underLow && adc1underLow && adc2underHigh) {
        return_val = (adcValues[0] + adcValues[1]) / 2;
        return RedundantADC::Status::OFF_BY_ONE_ERROR;
    }

    // Margin error check
    if (adc0underHigh) {
        return_val = adcValues[0];
        return RedundantADC::Status::MARGIN_ERROR;
    } else if (adc1underHigh) {
        return_val = adcValues[1];
        return RedundantADC::Status::MARGIN_ERROR;
    } else if (adc2underHigh) {
        return_val = adcValues[2];
        return RedundantADC::Status::MARGIN_ERROR;
    }

    // Default case
    return_val = 0;
    return RedundantADC::Status::COMPARISON_ERROR;
}

}// namespace RedundantADC
