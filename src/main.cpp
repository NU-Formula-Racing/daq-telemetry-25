#include <Arduino.h>
#include <RH_RF95.h>

#include <can.hpp>
#include <can_drivers.hpp>

#include "resources.hpp"

void setup() {
    can::ESPCANDriver<ESPCAN_DEFAULT_TX_PIN, ESPCAN_DEFAULT_RX_PIN> driver;
    can::CANBus bus(driver, can::CANBaudRate::CBR_500KBPS);
    
    bus.validateMessages();
    bus.initialize();
}

void loop() {}