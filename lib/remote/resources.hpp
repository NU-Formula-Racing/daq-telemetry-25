#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#include <SPI.h>

#include <can.hpp>
#include <can_drivers.hpp>
#include <define.hpp>
#include <sd_manager.hpp>
#include <tasks.hpp>

#ifdef APP_REMOTE

namespace remote {

class Resources {
   public:
    static Resources& instance() {
        static Resources r;
        return r;
    }

    static tasks::TaskScheduler& sched() { return Resources::instance().scheduler; }

    static can::CANBus& data() { return Resources::instance().dataBus; }

    static can::CANBus& drive() { return Resources::instance().driveBus; }

    static FileGuard file(const char* path, const char* mode, const bool create = false) {
        return FileGuard(Resources::instance()._sdManager, path, mode,
                         FileGaurdBehavior::FGB_CLOSE_ON_DESTRUCTION, create);
    }

    static FileGuard flushFile(const char* path, const char* mode, const bool create = false) {
        return FileGuard(Resources::instance()._sdManager, path, mode,
                         FileGaurdBehavior::FGB_FLUSH_ON_DESTRUCTION, create);
    }
    

   private:
    Resources();
    void operator=(Resources const& other) = delete;
    can::ESPCANDriver<ESPCAN_DEFAULT_TX_PIN, ESPCAN_DEFAULT_RX_PIN> _driveDriver;
    can::MCPCanDriver<HWPin::CAN_DATA_MCP_CS, VSPI> _dataDriver;
    SDManager _sdManager;

   public:
    can::CANBus dataBus;
    can::CANBus driveBus;
    tasks::TaskScheduler scheduler;
};

}  // namespace remote

#endif

#endif  // __RESOURCES_H__