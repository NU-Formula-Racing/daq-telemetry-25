#ifndef __TASK_LOG_H__
#define __TASK_LOG_H__

#include <Arduino.h>
#include <tasks.hpp>
#include <iostream>

namespace remote {

class LogTask : public tasks::TaskAction {
    bool initialize() {
        std::cout << "Starting Logging Task!\n"; 
        pinMode(HWPin::LOGGER_SD_CS, OUTPUT);
        digitalWrite(HWPin::LOGGER_SD_CS, HIGH);
        return true;
    }

    void run() {
        digitalWrite(HWPin::LOGGER_SD_CS, LOW);
        std::cout << "Running Logging Task!\n";
        Resources::sched().delayMs(10);
        digitalWrite(HWPin::LOGGER_SD_CS, HIGH);
    }

    void end() {

    }
};

} // namespace remote

#endif // __TASK_LOG_H__