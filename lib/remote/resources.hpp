#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#include <SPI.h>

#include <can.hpp>
#include <can_drivers.hpp>
#include <define.hpp>
#include <tasks.hpp>

#ifdef APP_REMOTE

namespace remote {

class Resources {
   public:
    static Resources& instance() {
        static Resources r;
        return r;
    }

    static tasks::TaskScheduler& sched() {
        return Resources::instance().scheduler;
    }

    static can::CANBus& data() {
        return Resources::instance().dataBus;
    }

    static can::CANBus& drive() {
        return Resources::instance().driveBus;
    }

   private:
    Resources();
    void operator=(Resources const& other) = delete;

    can::ESPCANDriver<
        ESPCAN_DEFAULT_TX_PIN,
        ESPCAN_DEFAULT_RX_PIN>
        _driveDriver;

    can::MCPCanDriver<
        HWPin::CAN_DATA_MCP_CS,
        VSPI>
        _dataDriver;

   public:
    can::CANBus dataBus;
    can::CANBus driveBus;
    tasks::TaskScheduler scheduler;
};

}  // namespace remote

#endif

#endif  // __RESOURCES_H__