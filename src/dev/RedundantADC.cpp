#include <EVT/io/ADC.hpp>
#include <dev/RedundantADC.hpp>

namespace IO = EVT::core::IO;

namespace RedundantADC {

/**
 * @brief Constructs a RedundantADC object with the given ADC instances.
 *
 * @param[in] adc0 The first ADC instance.
 * @param[in] adc1 The second ADC instance.
 * @param[in] adc2 The third ADC instance.
 */
RedundantADC::RedundantADC(IO::ADC& adc0, IO::ADC& adc1, IO::ADC& adc2) : adc0(adc0), adc1(adc1), adc2(adc2) {}

/**
 * @brief Process readings from the ADCs and check for redundancy.
 *
 * This function reads values from three ADCs, performs bitwise comparison,
 * and checks for redundancy among the readings.
 *
 * @param[in] val1 Reference to the variable to store the value read from the first ADC.
 * @param[in] val2 Reference to the variable to store the value read from the second ADC.
 * @param[in] val3 Reference to the variable to store the value read from the third ADC.
 * @return RedundantADC::Status The status of the processing.
 */
RedundantADC::Status RedundantADC::process(uint32_t& val1, uint32_t& val2, uint32_t& val3) {
    // Read ADC values
    uint32_t adc0ValueUint = static_cast<uint32_t>(adc0.read() * 1000);
    uint32_t adc1ValueUint = static_cast<uint32_t>(adc1.read() * 1000);
    uint32_t adc2ValueUint = static_cast<uint32_t>(adc2.read() * 1000);

    // Calculate differences
    int32_t diff01 = adc0ValueUint - adc1ValueUint;
    int32_t diff02 = adc0ValueUint - adc2ValueUint;
    int32_t diff12 = adc1ValueUint - adc2ValueUint;

    // Check for exact match
    if (diff01 == 0 || diff02 == 0 || diff12 == 0) {
        val1 = adc0ValueUint;
        val2 = adc1ValueUint;
        val3 = adc2ValueUint;
        return RedundantADC::Status::OK;
    } else if (diff01 == 1 || diff02 == 1 || diff12 == 1 || diff01 == -1 || diff02 == -1 || diff12 == -1) {
        // Check for one error
        return RedundantADC::Status::OFF_BY_ONE_ERROR;
    } else {
        // Calculate percentage difference
        uint32_t margin0 = (diff01 * 100) / adc0ValueUint;
        uint32_t margin1 = (diff02 * 100) / adc0ValueUint;
        uint32_t margin2 = (diff12 * 100) / adc1ValueUint;

        if (margin0 < 5 || margin1 < 5 || margin2 < 5) {
            return RedundantADC::Status::MARGIN_ERROR;
        } else {
            return RedundantADC::Status::COMPARISON_ERROR;
        }
    }
}

}// namespace RedundantADC
