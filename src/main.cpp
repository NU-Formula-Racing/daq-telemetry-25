#include <Arduino.h>
#include <RH_RF95.h>

#include <can.hpp>
#include <can_drivers.hpp>
#include "resources.hpp"

void setup() {
    can::ESPCANDriver<ESPCAN_DEFAULT_TX_PIN, ESPCAN_DEFAULT_RX_PIN> driver;
    can::CANBus bus(driver, can::CANBaudRate::CBR_500KBPS);
    
    bus.addMessage({.id = 0x123,
                    .length = 8,
                    .type = can::FrameType::STANDARD,
                    .signals = {{.startBit = 0,
                                 .length = 4,
                                 .isSigned = false,
                                 .factor = 1.0,
                                 .offset = 0.0,
                                 .minimum = 0.0,
                                 .maximum = 255.0},
                                {.startBit = 4,
                                 .length = 4,
                                 .isSigned = false,
                                 .endianness = can::Endianness::MSG_LITTLE_ENDIAN,
                                 .factor = 1.0,
                                 .offset = 0.0,
                                 .minimum = 0.0,
                                 .maximum = 255.0}

                    }});

    bus.addMessage(
        can::CANMessageDescription{
            .id = 0x456,
            .length = 8,
            .type = can::FrameType::STANDARD,
            .signals = {
                can::CANSignalDescription{
                    .startBit = 0,
                    .length = 4,
                    .isSigned = false,
                    .endianness = can::Endianness::MSG_LITTLE_ENDIAN,
                    .factor = 1.0,
                    .offset = 0.0,
                    .minimum = 0.0,
                    .maximum = 255.0
                },
                can::CANSignalDescription{
                    .startBit = 4,
                    .length = 4,
                    .isSigned = false,
                    .endianness = can::Endianness::MSG_LITTLE_ENDIAN,
                    .factor = 1.0,
                    .offset = 0.0,
                    .minimum = 0.0,
                    .maximum = 255.0
                }
            }
        }
    );

    bus.validateMessages();
    bus.initialize();
}

void loop() {}