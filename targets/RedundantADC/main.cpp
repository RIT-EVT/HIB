/**
*
* This file demonstrates a basic sample of using the RedundantADC module.
* The program initializes the system, sets up UART communication, configures ADC pins for throttle,
* creates a RedundantADC object, and continuously reads ADC values from three ADCs.
* It then compares the values to see if any of them match and prints the results.
*/

#include <EVT/dev/RTCTimer.hpp>
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

    // Setup ADC pins for throttle
    IO::ADC& adc0 = IO::getADC<IO::Pin::PA_0>();
    IO::ADC& adc1 = IO::getADC<IO::Pin::PA_1>();
    IO::ADC& adc2 = IO::getADC<IO::Pin::PA_2>();

    // Set up ADC pins for braking
    /*
    IO::ADC& b_adc0 = IO::getADC<IO::Pin::PA_4>();
    IO::ADC& b_adc1 = IO::getADC<IO::Pin::PA_5>();
    IO::ADC& b_adc2 = IO::getADC<IO::Pin::PA_6>();
    */

    // Create RedundantADC object
    RedundantADC::RedundantADC redundantADC(adc0, adc1, adc2);

    // Variables to store ADC values
    uint32_t val1, val2, val3;

    uint32_t adc_read_count = 0;

    DEV::RTC& clock = DEV::getRTC();
    DEV::RTCTimer timer(clock, 1000);

    while (1) {
        // 334 is a read every 3ms (minimally required)
        // 1000 is a read every 1ms (optimally fast)
        if (adc_read_count >= 334) {
            timer.stopTimer();
            uart.printf("ADC read count: %d\r\n", adc_read_count);
            uart.printf("Time taken: %d\r\n", timer.getTime());
        }
        // Process ADC values
        RedundantADC::RedundantADC::Status status = redundantADC.process(val1, val2, val3);
        // Print ADC values
        if (status == RedundantADC::RedundantADC::Status::OK) {
            adc_read_count++;
        } else {
            adc_read_count++;
        }
    }
}
