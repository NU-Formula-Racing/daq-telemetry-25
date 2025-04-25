#ifndef __TASK_CAN_H__
#define __TASK_CAN_H__

#include <tasks.hpp>
#include <iostream>

namespace remote {

class CANTask : public tasks::TaskAction {
    bool initialize() {
        std::cout << "Starting CAN Task!\n"; 
        return true;
    }

    void run() {
        std::cout << "Running CAN Task!\n"; 
    }

    void end() {

    }
};

} // namespace remote

#endif // __TASK_CAN_H__