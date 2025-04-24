#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#include <can.hpp>
#include <tasks.hpp>

namespace common {

class Resources {
   public:
    static Resources& instance() {
        static Resources r;
        return r;
    }

    static tasks::TaskScheduler &scheduler() {

    }

    static tasks::Tasks

   private:
    Resources();
    void operator=(Resources const& other) = delete;

   public:
    can::CANBus dataBus;
    can::CANBus driveBus;
    tasks::TaskScheduler scheduler;
};

}  // namespace common

#endif  // __RESOURCES_H__