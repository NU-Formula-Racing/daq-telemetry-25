#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#include <can.hpp>
#include <tasks.hpp>

#ifdef APP_REMOTE

namespace common {

class Resources {
   public:
    static Resources& instance() {
        static Resources r;
        return r;
    }

    static const tasks::TaskScheduler& sched() {
        return Resources::instance().scheduler;
    }

    static const can::CANBus &data() {
        return Resources::instance().dataBus;
    }

   private:
    Resources();
    void operator=(Resources const& other) = delete;

   public:
    can::CANBus dataBus;
    can::CANBus driveBus;
    tasks::TaskScheduler scheduler;
};

}  // namespace common

#endif

#endif  // __RESOURCES_H__