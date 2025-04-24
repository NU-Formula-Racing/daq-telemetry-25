#include <Arduino.h>
#include <RH_RF95.h>

#include <can.hpp>
#include <can_drivers.hpp>
#include <tasks.hpp>

#include "resources.hpp"

// resources initialization steps!
static void __setupCAN();
static void __setupTasks();

void setup() {
    __setupCAN();
    __setupTasks();
}

void loop() {}

static void __setupCAN() {
    can::ESPCANDriver<ESPCAN_DEFAULT_TX_PIN, ESPCAN_DEFAULT_RX_PIN> driver;
    can::CANBus bus(driver, can::CANBaudRate::CBR_500KBPS);

    bus.validateMessages();
    bus.initialize();
}

static void __setupTasks() {
    Resources::instance().
}