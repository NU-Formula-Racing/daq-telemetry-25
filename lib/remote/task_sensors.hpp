#ifndef __TASK_SENSORS_H__
#define __TASK_SENSORS_H__

#include <tasks.hpp>
#include <iostream>

#include <RTClib.h>

namespace remote {

class SensorsTask : public tasks::TaskAction {
    bool initialize() {
        std::cout << "Starting Sensors Task!\n"; 
        pinMode(HWPin::WS_TASK, OUTPUT);
        digitalWrite(HWPin::WS_TASK, LOW);
        return true;
    }

    void run() {
        digitalWrite(HWPin::WS_TASK, HIGH);
        std::cout << "Running Sensors Task!\n";
        Resources::sched().delayMs(10);
        digitalWrite(HWPin::WS_TASK, LOW); 
    }

    void end() {

    }
};

} // namespace remote

#endif // __TASK_SENSORS_H__