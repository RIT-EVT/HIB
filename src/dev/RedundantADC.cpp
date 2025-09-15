#include <dev/RedundantADC.hpp>
#include <dev/ADS8689IPWR.hpp>
#include <cmath>
#include <core/utils/log.hpp>

namespace io = core::io;

// Percantage differences
// Values are 1% and 5% of 12288 Mv respectively not really though
constexpr uint32_t LOW_MARGIN = 400;
constexpr uint32_t HIGH_MARGIN = 800;

namespace HIB::DEV {

RedundantADC::RedundantADC(ADS8689IPWR& adc0, ADS8689IPWR& adc1, ADS8689IPWR& adc2) : adc0(adc0), adc1(adc1), adc2(adc2) {}

RedundantADC::Status RedundantADC::read(uint32_t& return_val) const {
    // Read in the millivoltage of each ADC
    int32_t adcValues[3];
    adcValues[0] = static_cast<int32_t>(adc0.read());
    adcValues[1] = static_cast<int32_t>(adc1.read());
    adcValues[2] = static_cast<int32_t>(adc2.read());

    // Calculate average of all ADC millivoltages
    const int32_t average = (adcValues[0] + adcValues[1] + adcValues[2]) / 3;

    static int count = 0;
    if (count > 2000) {
        core::log::LOGGER.log(core::log::Logger::LogLevel::INFO, "Voltage average at the RedundantADC: %i\r\n", average);
        core::log::LOGGER.log(core::log::Logger::LogLevel::INFO, "Voltage average at the RedundantADC: %i\r\n", average);
        core::log::LOGGER.log(core::log::Logger::LogLevel::INFO, "Voltage average at the RedundantADC: %i\r\n", average);
        core::log::LOGGER.log(core::log::Logger::LogLevel::INFO, "Voltage average at the RedundantADC: %i\r\n", average);
        count = 0;
    }
    count += 1;

    // Check for deviation errors.
    // Formula: |DEVIATION_FROM_AVERAGE| / AVERAGE (NORMALIZED) * 100 TO SCALE UP PERCENTAGE FROM 0.01 TO 1
    const bool adc0underLow = static_cast<uint32_t>(std::abs(adcValues[0] - average)) < LOW_MARGIN;
    const bool adc1underLow = static_cast<uint32_t>(std::abs(adcValues[1] - average)) < LOW_MARGIN;
    const bool adc2underLow = static_cast<uint32_t>(std::abs(adcValues[2] - average)) < LOW_MARGIN;

    const bool adc0underHigh = static_cast<uint32_t>(std::abs(adcValues[0] - average)) < HIGH_MARGIN;
    const bool adc1underHigh = static_cast<uint32_t>(std::abs(adcValues[1] - average)) < HIGH_MARGIN;
    const bool adc2underHigh = static_cast<uint32_t>(std::abs(adcValues[2] - average)) < HIGH_MARGIN;

    // Check for redundancy
    const bool allUnderLow = adc0underLow && adc1underLow && adc2underLow;

    if (average == 0) {
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
    return RedundantADC::Status::COMPARISON_ERROR;
}

}// namespace HIB::DEV
