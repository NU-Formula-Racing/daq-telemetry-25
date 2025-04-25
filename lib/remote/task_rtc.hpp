#ifndef __TASK_RTC_H__
#define __TASK_RTC_H__

#include <tasks.hpp>
#include <iostream>

namespace remote {

class RTCTask : public tasks::TaskAction {
    bool initialize() {
        std::cout << "Starting RTC Task!\n"; 
        return true;
    }

    void run() {
        std::cout << "Running RTC Task!\n"; 
    }

    void end() {

    }
};

} // namespace remote

#endif // __TASK_RTC_H__