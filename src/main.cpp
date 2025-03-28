#include <Arduino.h>
#include <RH_RF95.h>

#include <can/can.hpp>
#include <can/can_driver_esp.hpp>

void setup() {
    can::ESPCANDriver<ESPCAN_DEFAULT_TX_PIN, ESPCAN_DEFAULT_RX_PIN> driver;
    can::CANBus bus(can::CANBaudRate::CAN_100KBPS, driver);

    bus.addMessage({.id = 0x123,
                    .length = 8,
                    .type = can::FrameType::STANDARD,
                    .signals = {{.start_bit = 0,
                                 .length = 4,
                                 .is_signed = false,
                                 .endianness = can::Endianness::MSG_LITTLE_ENDIAN,
                                 .factor = 1.0,
                                 .offset = 0.0,
                                 .minimum = 0.0,
                                 .maximum = 255.0},
                                {.start_bit = 4,
                                 .length = 4,
                                 .is_signed = false,
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
                    .start_bit = 0,
                    .length = 4,
                    .is_signed = false,
                    .endianness = can::Endianness::MSG_LITTLE_ENDIAN,
                    .factor = 1.0,
                    .offset = 0.0,
                    .minimum = 0.0,
                    .maximum = 255.0
                },
                can::CANSignalDescription{
                    .start_bit = 4,
                    .length = 4,
                    .is_signed = false,
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