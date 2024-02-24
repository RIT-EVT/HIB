/**
*
* This file demonstrates a basic sample of using the RedundantADC module.
* The program initializes the system, sets up UART communication, configures ADC pins for throttle,
* creates a RedundantADC object, and continuously reads ADC values from three ADCs.
* It then compares the values to see if any of them match and prints the results.
*/

#include <EVT/io/ADC.hpp>
#include <EVT/io/UART.hpp>
#include <EVT/io/pin.hpp>
#include <EVT/manager.hpp>
#include <dev/RedundantADC.hpp>

namespace IO = EVT::core::IO;
namespace DEV = EVT::core::DEV;

int main() {
    // Initialize system
    EVT::core::platform::init();

    // Setup UART
    IO::UART& uart = IO::getUART<IO::Pin::UART_TX, IO::Pin::UART_RX>(9600);
    uart.printf("UART initialized\r\n");

    IO::ADC& adc0 = IO::getADC<IO::Pin::PA_0>();
    uart.printf("ADC0 initialized\r\n");
    IO::ADC& adc1 = IO::getADC<IO::Pin::PA_1>();
    uart.printf("ADC1 initialized\r\n");
    IO::ADC& adc2 = IO::getADC<IO::Pin::PA_4>();
    uart.printf("ADC2 initialized\r\n");

    // Create RedundantADC object
    RedundantADC::RedundantADC redundantADC(adc0, adc1, adc2);

    // Variables to store ADC values
    uint32_t val1, val2, val3;

    while (1) {
        // Process ADC values
        RedundantADC::RedundantADC::Status status = redundantADC.readVoltage(val1, val2, val3);

        //check ADC Statuses0
 0       if (status == RedundantADC::RedundantADC::Status::OK) {
            uart.printf("ADC0: %d mV, ADC1: %d mV, ADC2: %d mV\r\n", val1, val2, val3);
        } else if (status == RedundantADC::RedundantADC::Status::OFF_BY_ONE_ERROR) {
            uart.printf("One error detected\r\n");
        } else if (status == RedundantADC::RedundantADC::Status::MARGIN_ERROR) {
            uart.printf("Margin error detected\r\n");
        } else if (status == RedundantADC::RedundantADC::Status::COMPARISON_ERROR) {
            uart.printf("Comparison error detected\r\n");
        }
    }
}
