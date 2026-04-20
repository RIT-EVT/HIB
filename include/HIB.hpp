#ifndef HIB_
#define HIB_

#include "core/io/types/CANMessage.hpp"
#include <cstddef>
#include <dev/RedundantADC.hpp>

namespace io = core::io;

namespace HIB {
/**
 * Id of the message being sent from the HIB
 */
#define HIB_MESSAGE_ID 0xD0

/**
 * Deadzone for the ADCs so they do not send a signal before the throttle is twisted
 */
#define VOLTAGE_DEADZONE 475

/**
 * Start Switch Pin
 */
#define START io::Pin::PB_1

/**
 * Throttle Switch Pin
 */
#define THROTTLE_SWITCH io::Pin::PB_2

/**
 * Forward Enable Pins
 */
#define FORWARD_ENABLE_1 io::Pin::PB_4
#define FORWARD_ENABLE_2 io::Pin::PB_5
#define FORWARD_ENABLE_3 io::Pin::PB_6

/**
 * Short circuit checking pins that should remain high unless there is a short
 */
#define NSHORT_1 io::Pin::PA_0
#define NSHORT_2 io::Pin::PA_1

/**
 * Open circuit checking pins that should remain high unless the circuit is open
 */
#define NOPEN_1 io::Pin::PA_8
#define NOPEN_2 io::Pin::PA_9

/**
 * Self test DAC output pins
 */
#define DAC_OUT_1 io::Pin::PA_4
#define DAC_OUT_2 io::Pin::PA_5

/**
 * Enable pins for each self test loop
 */
#define SELF_TEST_ENABLE_1 io::Pin::PA_6
#define SELF_TEST_ENABLE_2 io::Pin::PA_7

/**
 * JTAG serial wire pins
 */
#define SWDIO io::Pin::PA_13 // Serial wire debug input/output
#define SWO io::Pin::PB_3    // Serial wire output
#define SWDCLK io::Pin::PA_14// Serial wire clock

/**
 * ADC reset pins
 */
#define ADC_1_NRST io::Pin::PA_15
#define ADC_2_NRST io::Pin::PB_0

/**
 * Debug LED pins
 */
#define DEBUG_LED_1 io::Pin::PC_6
#define DEBUG_LED_2 io::Pin::PC_13

/**
 * Throttle ADC ready pins
 */
#define THROTTLE_RVS0 io::Pin::PC_3
#define THROTTLE_RVS1 io::Pin::PC_4
#define THROTTLE_RVS2 io::Pin::PC_5

/**
 * Brake ADC ready pins
 */
#define BRAKE_RVS0 io::Pin::PC_0
#define BRAKE_RVS1 io::Pin::PC_1
#define BRAKE_RVS2 io::Pin::PC_2

constexpr size_t payloadLength = 6;

typedef struct {
    io::GPIO* throttleSwitch = nullptr;
    io::GPIO* start = nullptr;
    io::GPIO* forwardEnable1 = nullptr;
    io::GPIO* forwardEnable2 = nullptr;
    io::GPIO* forwardEnable3 = nullptr;
    io::GPIO* nshort1 = nullptr;
    io::GPIO* nshort2 = nullptr;
    io::GPIO* nopen1 = nullptr;
    io::GPIO* nopen2 = nullptr;
    io::GPIO* throttleRvs0 = nullptr;
    io::GPIO* throttleRvs1 = nullptr;
    io::GPIO* throttleRvs2 = nullptr;
    io::GPIO* brakeRvs0 = nullptr;
    io::GPIO* brakeRvs1 = nullptr;
    io::GPIO* brakeRvs2 = nullptr;
    io::GPIO* debugLed1 = nullptr;
    io::GPIO* debugLed2 = nullptr;
    io::GPIO* swdio = nullptr;
    io::GPIO* swo = nullptr;
    io::GPIO* swdclk = nullptr;
    io::GPIO* adcNrst1 = nullptr;
    io::GPIO* adcNrst2 = nullptr;
    io::GPIO* dacOut1 = nullptr;
    io::GPIO* dacOut2 = nullptr;
    io::GPIO* selfTestEnable1 = nullptr;
    io::GPIO* selfTestEnable2 = nullptr;
} HibPinMap;

/**
 * The Handlebar Interface Board Takes in a 0.0 to 12.0 volt signal from the throttle and brake
 * which it then converts to a 16 bit value through its "double-triple" ADC setup. It then sends
 * this voltage value along with any error codes that were generated to the Vehicle Control Unit
 * (VCU).
 *
 * It reads in the voltage from either the throttle or the brake through one set of three ADCs
 * that communicate with the microcontroller using SPI. After the controller sets up the devices
 * they are read in and then averaged and contrasted with the average to find any margin errors.
 *
 * The data after being correctly parsed and checked for errors is then sent through CAN to the
 * VCU for decision-making.
 */
class HIB {
public:
    HIB(RedundantADC& throttle, RedundantADC& brake, HibPinMap pinMap);

    HIB(RedundantADC& throttle, RedundantADC& brake);

    /**
     * Reads the voltages and errors from the redundant ADCs and returns a can message containing
     * the data from each read
     * @return The can message that should be transmitted
     */
    io::CANMessage process();

    /**
     * Payload for CAN message transmission.
     * Byte 0: MSB for Throttle Voltage
     * Byte 1: LSB for Throttle Voltage
     * Byte 2: MSB for Brake Voltage
     * Byte 3: LSB for Brake Voltage
     * Byte 4: Error Byte for Throttle
     * Byte 5: Error Byte for Brake
     * Error Byte Layout: 0: No Error, 1: Precision Error, 2: Margin Error, 3: Comparison Error
     */
    struct {
        uint8_t throttleVoltageMSB = 0;
        uint8_t throttleVoltageLSB = 0;
        uint8_t brakeVoltageMSB = 0;
        uint8_t brakeVoltageLSB = 0;
        uint8_t throttleError = 0;
        uint8_t brakeError = 0;
    } hibPayload;

    HibPinMap pinMap;

private:
    /**
     * Reads the 0V-12V signal coming from the throttle potentiometer
     */
    void readThrottleVoltage();

    /**
     * Do nothing because there is no brake setup yet
     */
    void readBrakeVoltage();

    RedundantADC& throttle;
    RedundantADC& brake;

    uint16_t throttleVoltage = 0;
    uint16_t brakeVoltage = 0;

    // Counters for throttle errors
    uint32_t acceptableThrottleMarginErrors = 0;
    uint32_t precisionThrottleMarginErrors = 0;
    uint32_t comparisonThrottleErrors = 0;

    // Counters for brake errors
    uint32_t acceptableBrakeMarginErrors = 0;
    uint32_t precisionBrakeMarginErrors = 0;
    uint32_t comparisonBrakeErrors = 0;
};
}// namespace HIB
#endif