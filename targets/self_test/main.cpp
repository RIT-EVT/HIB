/**
 * Self test main file for the HIB. This will pull the self test pins
 */
#include <HIB.hpp>
#include <core/io/GPIO.hpp>
#include <core/io/SPI.hpp>
#include <core/io/UART.hpp>
#include <core/manager.hpp>
#include <core/utils/log.hpp>
#include <dev/ADS8689IPWR.hpp>
#include <dev/RedundantADC.hpp>

namespace io = core::io;
namespace time = core::time;

constexpr uint8_t deviceCount = 3;

io::GPIO* throttleDevices[deviceCount];
io::GPIO* brakeDevices[deviceCount];

int main() {
    // Initialize system
    core::platform::init();

    // Setup UART
    io::UART& uart = io::getUART<io::Pin::UART_TX, io::Pin::UART_RX>(9600);
    core::log::LOGGER.setUART(&uart);
    core::log::LOGGER.setLogLevel(core::log::Logger::LogLevel::INFO);

    // Setup the pin mapping
    HIB::HibPinMap pinMap{
        // Setup GPIO pins for the throttle switch, start, and forward enable to check before
        // processing and sending anything to the VCU. (If any of these are false, then the
        // throttle should be cut)
        .throttleSwitch = &io::getGPIO<THROTTLE_SWITCH>(io::GPIO::Direction::INPUT),
        .start = &io::getGPIO<START>(io::GPIO::Direction::INPUT),
        .forwardEnable1 = &io::getGPIO<FORWARD_ENABLE_1>(io::GPIO::Direction::INPUT),
        .forwardEnable2 = &io::getGPIO<FORWARD_ENABLE_2>(io::GPIO::Direction::INPUT),
        .forwardEnable3 = &io::getGPIO<FORWARD_ENABLE_3>(io::GPIO::Direction::INPUT),

        // Short and open check pins should he high until they are pulled low when an error occurs
        .nshort1 = &io::getGPIO<NSHORT_1>(io::GPIO::Direction::INPUT),
        .nshort2 = &io::getGPIO<NSHORT_2>(io::GPIO::Direction::INPUT),
        .nopen1 = &io::getGPIO<NOPEN_1>(io::GPIO::Direction::INPUT),
        .nopen2 = &io::getGPIO<NOPEN_2>(io::GPIO::Direction::INPUT),

        // Throttle RVS pins for checking its activation state
        .throttleRvs0 = &io::getGPIO<THROTTLE_RVS0>(io::GPIO::Direction::INPUT),
        .throttleRvs1 = &io::getGPIO<THROTTLE_RVS1>(io::GPIO::Direction::INPUT),
        .throttleRvs2 = &io::getGPIO<THROTTLE_RVS2>(io::GPIO::Direction::INPUT),

        // Brake RVS pins for checking its activation state
        .brakeRvs0 = &io::getGPIO<BRAKE_RVS0>(io::GPIO::Direction::INPUT),
        .brakeRvs1 = &io::getGPIO<BRAKE_RVS1>(io::GPIO::Direction::INPUT),
        .brakeRvs2 = &io::getGPIO<BRAKE_RVS2>(io::GPIO::Direction::INPUT),

        // Debug LED pins for testing
        .debugLed1 = &io::getGPIO<DEBUG_LED_1>(io::GPIO::Direction::OUTPUT),
        .debugLed2 = &io::getGPIO<DEBUG_LED_1>(io::GPIO::Direction::OUTPUT),

        // Serial I/O pins
        .swdio = &io::getGPIO<SWDIO>(io::GPIO::Direction::OUTPUT),
        .swo = &io::getGPIO<SWO>(io::GPIO::Direction::OUTPUT),
        .swdclk = &io::getGPIO<SWDCLK>(io::GPIO::Direction::OUTPUT),

        // ADC reset pins
        // TODO: Name them better according to which ADC setup they are connected to
        .adcNrst1 = &io::getGPIO<ADC_1_NRST>(io::GPIO::Direction::OUTPUT),
        .adcNrst2 = &io::getGPIO<ADC_2_NRST>(io::GPIO::Direction::OUTPUT),

        // TODO: Name them better according to which ADC setup they are connected to
        .dacOut1 = &io::getGPIO<DAC_OUT_1>(io::GPIO::Direction::INPUT),
        .dacOut2 = &io::getGPIO<DAC_OUT_2>(io::GPIO::Direction::INPUT),

        // TODO: Name them better according to which ADC setup they are connected to
        .selfTestEnable1 = &io::getGPIO<SELF_TEST_ENABLE_1>(io::GPIO::Direction::OUTPUT),
        .selfTestEnable2 = &io::getGPIO<SELF_TEST_ENABLE_2>(io::GPIO::Direction::OUTPUT),
    };

    pinMap.nshort1->writePin(io::GPIO::State::HIGH);
    pinMap.nshort2->writePin(io::GPIO::State::HIGH);
    pinMap.nopen1->writePin(io::GPIO::State::HIGH);
    pinMap.nopen2->writePin(io::GPIO::State::HIGH);

    // Set up each chip select pin for the brake and throttle
    brakeDevices[0] = &io::getGPIO<BRAKE_0>(io::GPIO::Direction::OUTPUT);
    brakeDevices[0]->writePin(io::GPIO::State::HIGH);
    brakeDevices[1] = &io::getGPIO<BRAKE_1>(io::GPIO::Direction::OUTPUT);
    brakeDevices[1]->writePin(io::GPIO::State::HIGH);
    brakeDevices[2] = &io::getGPIO<BRAKE_2>(io::GPIO::Direction::OUTPUT);
    brakeDevices[2]->writePin(io::GPIO::State::HIGH);

    throttleDevices[0] = &io::getGPIO<THROTTLE_0>(io::GPIO::Direction::OUTPUT);
    throttleDevices[0]->writePin(io::GPIO::State::HIGH);
    throttleDevices[1] = &io::getGPIO<THROTTLE_1>(io::GPIO::Direction::OUTPUT);
    throttleDevices[1]->writePin(io::GPIO::State::HIGH);
    throttleDevices[2] = &io::getGPIO<THROTTLE_2>(io::GPIO::Direction::OUTPUT);
    throttleDevices[2]->writePin(io::GPIO::State::HIGH);

    // Create the two SPI buses
    io::SPI& spiThrottle = io::getSPI<THROTTLE_SPI_SCK, THROTTLE_SPI_MOSI, THROTTLE_SPI_MISO>(throttleDevices, deviceCount);
    spiThrottle.configureSPI(SPI_SPEED, SPI_MODE, SPI_MSB_FIRST);
    io::SPI& spiBrake = io::getSPI<BRAKE_SPI_SCK, BRAKE_SPI_MOSI, BRAKE_SPI_MISO>(brakeDevices, deviceCount);
    spiBrake.configureSPI(SPI_SPEED, SPI_MODE, SPI_MSB_FIRST);

    // Create all 6 ADS8689IPWR objects
    auto throttleAdc1 = HIB::ADS8689IPWR(spiThrottle, 0);
    auto throttleAdc2 = HIB::ADS8689IPWR(spiThrottle, 1);
    auto throttleAdc3 = HIB::ADS8689IPWR(spiThrottle, 2);
    auto brakeAdc1 = HIB::ADS8689IPWR(spiBrake, 0);
    auto brakeAdc2 = HIB::ADS8689IPWR(spiBrake, 1);
    auto brakeAdc3 = HIB::ADS8689IPWR(spiBrake, 2);

    // Initialize the 2 redundant ADCs
    auto throttle = HIB::RedundantADC(throttleAdc1, throttleAdc2, throttleAdc3);
    auto brake = HIB::RedundantADC(brakeAdc1, brakeAdc2, brakeAdc3);

    // Initialize the HIB object to begin processing data
    auto hib = HIB::HIB(throttle, brake, pinMap);

    // Check the ready pins

    // Initialize CAN
    io::CAN& can = io::getCAN<CAN_TX, CAN_RX>(true);

    // Try to join the network
    io::CAN::CANStatus result = can.connect();
    if (result != io::CAN::CANStatus::OK) {
        LOG_INFO("Failed to connect to the CAN network.\r\n");
    }

    // ID for HIB is 0x0D0
    io::CANMessage transmit_message(0x0D0, 0, {}, false);

    // Try to send the message
    result = can.transmit(transmit_message);
    if (result != io::CAN::CANStatus::OK) {
        LOG_INFO("Failed to transmit message\r\n");
    } else {
        LOG_INFO("Transmitted message.\r\n");
    }

    // Read voltage and errors and send them through the CAN bus
    while (true) {
        // Do not process the throttle unless the bike is set up
        if (hib.pinMap.start->readPin() == io::GPIO::State::LOW 
			|| hib.pinMap.throttleSwitch->readPin() == io::GPIO::State::LOW 
			|| hib.pinMap.forwardEnable1->readPin() == io::GPIO::State::LOW 
			|| hib.pinMap.forwardEnable2->readPin() == io::GPIO::State::LOW 
			|| hib.pinMap.forwardEnable3->readPin() == io::GPIO::State::LOW) {
            continue;
        }

        // Process the voltage
        transmit_message = hib.process();

        // Try to send the message
        result = can.transmit(transmit_message);
        if (result != io::CAN::CANStatus::OK) {
            LOG_INFO("Failed to transmit message\r\n");
        }

        // Uncomment to print results through UART when a P-CAN dongle is unavailable
        // io::CANMessage message;
        // auto status = can.receive(&message);
        // uint8_t* payload = message.getPayload();
        // LOG_INFO("Throttle Voltage: %imV\r\n"
        //          "Brake Voltage: %imV\r\n"
        //          "Throttle Error Information: %x\r\n",
        //          "Brake Error Information: %x\r\n",
        //          (payload[0] << 8) + payload[1],
        //          (payload[2] << 8) + payload[3],
        //          payload[4],
        //          payload[5]);
        // time::wait(5000);
    }

    return -1;
}
