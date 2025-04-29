#ifndef __TASK_WIRELESS_H__
#define __TASK_WIRELESS_H__

#include <Arduino.h>
#include <tasks.hpp>
#include <iostream>
#include <define.hpp>

namespace remote {

class WirelessTask : public tasks::TaskAction {
    bool initialize() { 
        pinMode(HWPin::WLS_LORA_CS, OUTPUT);
        digitalWrite(HWPin::WLS_LORA_CS, HIGH);
        return true;
    }

    void run() {
        digitalWrite(HWPin::WLS_LORA_CS, LOW);
        Resources::sched().delayMs(10);
        digitalWrite(HWPin::WLS_LORA_CS, HIGH);
    }

    void end() {

    }
};

} // namespace remote

#endif // __TASK_WIRELESS_H__