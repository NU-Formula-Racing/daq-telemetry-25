#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#include <can/can.hpp>

class Resources {
   public:
    static Resources& instance() {
        static Resources r;
        return r;
    }

   private:
    Resources();
    void operator=(Resources const& other) = delete;

   public:
   can::CANBus dataBus;
   can::CANBus driveBus;
};

#endif  // __RESOURCES_H__