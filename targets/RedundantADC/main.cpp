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
    IO::ADC& adc1 = IO::getADC<IO::Pin::PA_1>();
    IO::ADC& adc2 = IO::getADC<IO::Pin::PA_4>();

    // Create RedundantADC object
    HIB::DEV::RedundantADC redundantADC(adc0, adc1, adc2);

    // Variables to store ADC values
    uint32_t return_val;

    while (1) {
        // Process ADC values
        HIB::DEV::RedundantADC::Status status = redundantADC.readVoltage(return_val);

        //check ADC Statuses0
        if (status == HIB::DEV::RedundantADC::Status::OK) {
            uart.printf("Average Voltage Reading %d\r\n", return_val);
        } else if (status == HIB::DEV::RedundantADC::Status::PRECISION_MARGIN_EXCEEDED) {
            uart.printf("One error detected\r\n");
        } else if (status == HIB::DEV::RedundantADC::Status::ACCEPTABLE_MARGIN_EXCEEDED) {
            uart.printf("Margin error detected\r\n");
        } else if (status == HIB::DEV::RedundantADC::Status::COMPARISON_ERROR) {
            uart.printf("Comparison error detected\r\n");
        }
    }
}
