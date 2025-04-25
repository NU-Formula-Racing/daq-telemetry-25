#ifndef __TASK_LOG_H__
#define __TASK_LOG_H__

#include <tasks.hpp>
#include <iostream>

namespace remote {

class LogTask : public tasks::TaskAction {
    bool initialize() {
        std::cout << "Starting Logging Task!\n"; 
        return true;
    }

    void run() {
        std::cout << "Running Logging Task!\n"; 
    }

    void end() {

    }
};

} // namespace remote

#endif // __TASK_LOG_H__