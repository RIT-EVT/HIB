/**
 * REV3-HIB main target. Runs the setup for the SPI, UART, and CAN buses. Creates the ADS8689IPWR,
 * RedundantADC, and HIB objects and then runs the hib.process(). Optionally logs out the data stream
 * from the HIB if the logger is enabled.
 *
 * The goal of this device is to read in voltage data from the throttle and the brake through a pair
 * of three redundant 16 bit ADCs. These readings are then compared to each other to find the
 * average voltage and any substantial errors or offset in the readings of each device. If the
 * errors are too great, then the HIB will send an error signal to the VCU, otherwise it just
 * data pertaining to the voltage of the break and the throttle
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

    // // Setup GPIO pins for the throttle switch, start, and forward enable
    // auto throttleSwitch =  &io::getGPIO<THROTTLE_SWITCH>(io::GPIO::Direction::INPUT);
    // auto start = &io::getGPIO<START>(io::GPIO::Direction::INPUT);
    // auto forwardEnable0 = &io::getGPIO<FORWARD_ENABLE_0>(io::GPIO::Direction::INPUT);
    // auto forwardEnable1 = &io::getGPIO<FORWARD_ENABLE_1>(io::GPIO::Direction::INPUT);
    // auto forwardEnable2 = &io::getGPIO<FORWARD_ENABLE_2>(io::GPIO::Direction::INPUT);

    // Set up each chip select pink
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
    auto hib = HIB::HIB(throttle, brake);

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
    while(true) {
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

    return 0;
}