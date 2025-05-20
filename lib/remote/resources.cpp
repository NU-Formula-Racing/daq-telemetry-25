#include "resources.hpp"

#include <can.h>

#include <can_drivers.hpp>
#include <define.hpp>
#include <sd_manager.hpp>
#include <tasks.hpp>

using namespace remote;

static const SDManagerConfig __SD_CONFIG = {
    .spi = SPI,
    .csPin = HWPin::LOGGER_SD_CS,
};

Resources::Resources()
    : _driveDriver(),
      driveBus(_driveDriver, can::CANBaudRate::CBR_500KBPS),
      _dataDriver(),
      dataBus(_dataDriver, can::CANBaudRate::CBR_500KBPS),
      _sdManager(__SD_CONFIG) {
    _sdManager.initialize();
}