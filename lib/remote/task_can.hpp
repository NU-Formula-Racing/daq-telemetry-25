#ifndef __TASK_CAN_H__
#define __TASK_CAN_H__

#include <Arduino.h>
#include <tasks.hpp>
#include <iostream>

namespace remote {

class CANTask : public tasks::TaskAction {
    bool initialize() {
        pinMode(HWPin::CAN_DATA_MCP_CS, OUTPUT);
        digitalWrite(HWPin::CAN_DATA_MCP_CS, HIGH);
        return true;
    }

    void run() {
        digitalWrite(HWPin::CAN_DATA_MCP_CS, LOW);
        Resources::sched().delayMs(10);
        digitalWrite(HWPin::CAN_DATA_MCP_CS, HIGH);
    }

    void end() {

    }
};

} // namespace remote

#endif // __TASK_CAN_H__