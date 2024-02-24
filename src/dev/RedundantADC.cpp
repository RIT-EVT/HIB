#include <EVT/io/ADC.hpp>
#include <dev/RedundantADC.hpp>

namespace IO = EVT::core::IO;

constexpr uint32_t MARGIN = 5;

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
RedundantADC::Status RedundantADC::readVoltage(uint32_t& val1, uint32_t& val2, uint32_t& val3) {
    // Read ADC values
    auto adc0ValueUint = static_cast<int32_t>(adc0.read() * 1000);
    auto adc1ValueUint = static_cast<int32_t>(adc1.read() * 1000);
    auto adc2ValueUint = static_cast<int32_t>(adc2.read() * 1000);

    // Calculate differences
    int32_t diff01 = adc0ValueUint - adc1ValueUint;
    int32_t diff02 = adc0ValueUint - adc2ValueUint;
    int32_t diff12 = adc1ValueUint - adc2ValueUint;

    // Find maximum absolute difference manually
    int32_t max_diff = diff01;
    if (diff02 > max_diff)
        max_diff = diff02;
    if (diff12 > max_diff)
        max_diff = diff12;

    if (max_diff == 0) {// Exact match
        val1 = adc0ValueUint;
        val2 = adc1ValueUint;
        val3 = adc2ValueUint;
        return RedundantADC::Status::OK;
    } else if (max_diff == 1 || max_diff == -1) {// Off by one error
        return RedundantADC::Status::OFF_BY_ONE_ERROR;
    } else {
        // Calculate percentage difference
        int32_t margin0 = (diff01 * 100) / ((adc0ValueUint + adc1ValueUint) / 2);
        int32_t margin1 = (diff02 * 100) / ((adc0ValueUint + adc2ValueUint) / 2);
        int32_t margin2 = (diff12 * 100) / ((adc1ValueUint + adc2ValueUint) / 2);

        if (margin0 < MARGIN && margin1 < MARGIN && margin2 < MARGIN) {
            return RedundantADC::Status::MARGIN_ERROR;
        } else {
            return RedundantADC::Status::COMPARISON_ERROR;
        }
    }
}

}// namespace RedundantADC
