#pragma once

#include <EVT/io/ADC.hpp>
#include <cstdint>

namespace IO = EVT::core::IO;

namespace RedundantADC {

class RedundantADC {
public:
    enum class Status {
        OK = 0,              // no error
        COMPARISON_ERROR = 1,// comparison error (none of the values match)
    };

    RedundantADC(IO::ADC& adc0, IO::ADC& adc1, IO::ADC& adc2);
    RedundantADC::Status process(IO::ADC& adc0, IO::ADC& adc1, IO::ADC& adc2, uint32_t& val1, uint32_t& val2, uint32_t& val3);

private:
    IO::ADC& adc0;
    IO::ADC& adc1;
    IO::ADC& adc2;
};

}// namespace RedundantADC
