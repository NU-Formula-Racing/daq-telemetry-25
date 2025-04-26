#ifndef __TASK_SENSORS_H__
#define __TASK_SENSORS_H__

#include <tasks.hpp>
#include <iostream>

#include <RTClib.h>

namespace remote {

class SensorsTask : public tasks::TaskAction {
    bool initialize() {
        std::cout << "Starting Sensors Task!\n"; 
        return true;
    }

    void run() {
        std::cout << "Running Sensors Task!\n"; 
    }

    void end() {

    }

    
};

} // namespace remote

#endif // __TASK_SENSORS_H__