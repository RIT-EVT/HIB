#include <EVT/io/ADC.hpp>
#include <dev/RedundantADC.hpp>

namespace IO = EVT::core::IO;

namespace RedundantADC {

RedundantADC::RedundantADC(IO::ADC& adc0, IO::ADC& adc1, IO::ADC& adc2) : adc0(adc0), adc1(adc1), adc2(adc2) {}

RedundantADC::Status RedundantADC::process(uint32_t& val1, uint32_t& val2, uint32_t& val3) {

    // main comparison logic

    uint32_t adc0ValueUint = static_cast<uint32_t>(adc0.read() * 1000);
    uint32_t adc1ValueUint = static_cast<uint32_t>(adc1.read() * 1000);
    uint32_t adc2ValueUint = static_cast<uint32_t>(adc2.read() * 1000);

    // bitwise comparison logic to check if any of the values match
    bool adc0_adc1_match = (adc0ValueUint & adc1ValueUint) || ((adc0ValueUint | adc1ValueUint) == adc0ValueUint);
    bool adc0_adc2_match = (adc0ValueUint & adc2ValueUint) || ((adc0ValueUint | adc2ValueUint) == adc0ValueUint);
    bool adc1_adc2_match = (adc1ValueUint & adc2ValueUint) || ((adc1ValueUint | adc2ValueUint) == adc1ValueUint);

    if ((adc0_adc1_match && adc0_adc2_match) || (adc0_adc1_match && adc1_adc2_match) || (adc0_adc2_match && adc1_adc2_match)) {
        val1 = adc0ValueUint;
        val2 = adc1ValueUint;
        val3 = adc2ValueUint;
        return RedundantADC::Status::OK;
    } else {
        return RedundantADC::Status::COMPARISON_ERROR;
    }

}

}// namespace RedundantADC
