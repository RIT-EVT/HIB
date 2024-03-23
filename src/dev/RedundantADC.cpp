#include <EVT/io/ADC.hpp>
#include <dev/RedundantADC.hpp>
#include <cmath>


namespace IO = EVT::core::IO;

constexpr uint32_t LOW_MARGIN = 1;
constexpr uint32_t HIGH_MARGIN = 5;

namespace HIB::DEV {


RedundantADC::RedundantADC(IO::ADC& adc0, IO::ADC& adc1, IO::ADC& adc2) : adc0(adc0), adc1(adc1), adc2(adc2) {}

RedundantADC::Status RedundantADC::readVoltage(uint32_t& return_val) {
    // Read ADC values
    int32_t adcValues[3];
    adcValues[0] = static_cast<int32_t>(adc0.read() * 1000);
    adcValues[1] = static_cast<int32_t>(adc1.read() * 1000);
    adcValues[2] = static_cast<int32_t>(adc2.read() * 1000);

    // Calculate average of all ADC values
    int32_t average = (adcValues[0] + adcValues[1] + adcValues[2]) / 3;

    // Check for margin error
    bool adc0underLow = (std::abs(adcValues[0] - average) * 100 / average) < LOW_MARGIN;
    bool adc1underLow = (std::abs(adcValues[1] - average) * 100 / average) < LOW_MARGIN;
    bool adc2underLow = (std::abs(adcValues[2] - average) * 100 / average) < LOW_MARGIN;

    bool adc0underHigh = (std::abs(adcValues[0] - average) * 100 / average) > HIGH_MARGIN;
    bool adc1underHigh = (std::abs(adcValues[1] - average) * 100 / average) > HIGH_MARGIN;
    bool adc2underHigh = (std::abs(adcValues[2] - average) * 100 / average) > HIGH_MARGIN;

    // Check for redundancy
    bool allUnderLow = adc0underLow && adc1underLow && adc2underLow;


    if (allUnderLow) {
        return_val = average;
        return RedundantADC::Status::OK;
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

    // Margin error check (if only one ADC is under high margin)
    if (adc0underHigh && adc1underHigh && adc2underLow) {
        return_val = adcValues[2];
        return RedundantADC::Status::MARGIN_ERROR;
    } else if (adc0underHigh && adc1underLow && adc2underHigh) {
        return_val = adcValues[1];
        return RedundantADC::Status::MARGIN_ERROR;
    } else if (adc0underLow && adc1underHigh && adc2underHigh) {
        return_val = adcValues[0];
        return RedundantADC::Status::MARGIN_ERROR;
    }

    return_val = 0;
    return RedundantADC::Status::COMPARISON_ERROR;
}

}// namespace HIB::DEV
