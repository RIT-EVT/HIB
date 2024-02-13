/**
* This is a target file for the RedundantADC target. It is a basic sample of using the ADC module. The program checks 3 ADC values and compares them to see if any of the values match.
*/

#include <EVT/io/ADC.hpp>
#include <EVT/io/UART.hpp>
#include <EVT/io/pin.hpp>
#include <EVT/manager.hpp>
#include <dev/RedundantADC.hpp>

namespace IO = EVT::core::IO;

int main() {
    // Initialize system
    EVT::core::platform::init();

    IO::UART& uart = IO::getUART<IO::Pin::UART_TX, IO::Pin::UART_RX>(9600);

    // Setup ADC pins for throttle
    IO::ADC& adc0 = IO::getADC<IO::Pin::PA_0>();
    IO::ADC& adc1 = IO::getADC<IO::Pin::PA_1>();
    IO::ADC& adc2 = IO::getADC<IO::Pin::PA_2>();

    // Create RedundantADC object
    RedundantADC::RedundantADC redundantADC(adc0, adc1, adc2);

    // Variables to store ADC values
    uint32_t val1, val2, val3;

    while (1) {
        // Process ADC values
        RedundantADC::RedundantADC::Status status = redundantADC.process(adc0, adc1, adc2, val1, val2, val3);

        // Print ADC values
        if (status == RedundantADC::RedundantADC::Status::OK) {
            uart.printf("ADC0: %d, ADC1: %d, ADC2: %d\n\r", val1, val2, val3);
        } else {
            uart.printf("Comparison error\n\r");
        }
    }
}