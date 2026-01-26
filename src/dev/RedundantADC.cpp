#include <cmath>
#include <core/utils/log.hpp>

#include <dev/ADS8689IPWR.hpp>
#include <dev/RedundantADC.hpp>

namespace io = core::io;
using namespace HIB;

/* Percentage differences
 * Values are approximately 1% and 5% of 12288 Mv, but "approximately" is a stretch.
 * These can be altered as needed in the future after stress testing the ADCs and finding out
 * how they perform while running on the bike
 */
constexpr uint16_t LOW_MARGIN = 400;
constexpr uint16_t HIGH_MARGIN = 800;

RedundantADC::RedundantADC(ADS8689IPWR& adc0, ADS8689IPWR& adc1, ADS8689IPWR& adc2)
    : adc0(adc0), adc1(adc1), adc2(adc2) {}

RedundantADC::Status RedundantADC::read(uint16_t& return_val) const {
    // Read in the millivoltage of each ADC
    int16_t adcValues[3] = {0};
    adcValues[0] = static_cast<int16_t>(adc0.readVoltage());
    adcValues[1] = static_cast<int16_t>(adc1.readVoltage());
    adcValues[2] = static_cast<int16_t>(adc2.readVoltage());

    // Calculate average of all ADC millivoltages
    const auto average = static_cast<uint16_t>((adcValues[0] + adcValues[1] + adcValues[2]) / 3);

    // Check for deviation errors
    const bool adc0underLow = static_cast<uint16_t>(std::abs(adcValues[0] - average)) < LOW_MARGIN;
    const bool adc1underLow = static_cast<uint16_t>(std::abs(adcValues[1] - average)) < LOW_MARGIN;
    const bool adc2underLow = static_cast<uint16_t>(std::abs(adcValues[2] - average)) < LOW_MARGIN;

    const bool adc0underHigh = static_cast<uint16_t>(std::abs(adcValues[0] - average)) < HIGH_MARGIN;
    const bool adc1underHigh = static_cast<uint16_t>(std::abs(adcValues[1] - average)) < HIGH_MARGIN;
    const bool adc2underHigh = static_cast<uint16_t>(std::abs(adcValues[2] - average)) < HIGH_MARGIN;

    // Check for redundancy
    const bool allUnderLow = adc0underLow && adc1underLow && adc2underLow;

    if (average == 0) {
        return_val = average;
        return RedundantADC::Status::OK;
    }

    if (allUnderLow) {
        return_val = average;
        return RedundantADC::Status::OK;
    }

    // Off by one error check
    if (adc0underHigh && adc1underLow && adc2underLow) {
        return_val = (adcValues[1] + adcValues[2]) / 2;
        return RedundantADC::Status::PRECISION_MARGIN_EXCEEDED;
    } else if (adc0underLow && adc1underHigh && adc2underLow) {
        return_val = (adcValues[0] + adcValues[2]) / 2;
        return RedundantADC::Status::PRECISION_MARGIN_EXCEEDED;
    } else if (adc0underLow && adc1underLow && adc2underHigh) {
        return_val = (adcValues[0] + adcValues[1]) / 2;
        return RedundantADC::Status::PRECISION_MARGIN_EXCEEDED;
    }

    // Margin error check (if only one ADC is under high margin)
    if (adc0underHigh && adc1underHigh && adc2underLow) {
        return_val = adcValues[2];
        return RedundantADC::Status::ACCEPTABLE_MARGIN_EXCEEDED;
    } else if (adc0underHigh && adc1underLow && adc2underHigh) {
        return_val = adcValues[1];
        return RedundantADC::Status::ACCEPTABLE_MARGIN_EXCEEDED;
    } else if (adc0underLow && adc1underHigh && adc2underHigh) {
        return_val = adcValues[0];
        return RedundantADC::Status::ACCEPTABLE_MARGIN_EXCEEDED;
    }

    return_val = 0;

    // Zero out adc's
    adcValues[0] = 0;
    adcValues[1] = 0;
    adcValues[2] = 0;

    return RedundantADC::Status::COMPARISON_ERROR;
}
