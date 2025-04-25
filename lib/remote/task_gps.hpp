#ifndef __TASK_GPS_H__
#define __TASK_GPS_H__

#include <tasks.hpp>
#include <iostream>

namespace remote {

class GPSTask : public tasks::TaskAction {
    bool initialize() {
        std::cout << "Starting GPS Task!\n"; 
        return true;
    }

    void run() {
        std::cout << "Running GPS Task!\n"; 
    }

    void end() {

    }
};

} // namespace remote

#endif // __TASK_GPS_H__