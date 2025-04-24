#include <Arduino.h>
#include <RH_RF95.h>

#include <can.hpp>
#include <can_drivers.hpp>
#include <tasks.hpp>

#include "resources.hpp"

// resources initialization steps!
static void __setupTasks();

void setup() {
    __setupTasks();
}

void loop() {
}

static void __setupTasks() {
}