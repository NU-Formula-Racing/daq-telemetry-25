#include "resources.hpp"

#include <can.h>

#include <can_drivers.hpp>
#include <tasks.hpp>

using namespace remote;

Resources::Resources() : _driveDriver(),
                         driveBus(_driveDriver, can::CANBaudRate::CBR_500KBPS),
                         _dataDriver(),
                         dataBus(_dataDriver, can::CANBaudRate::CBR_500KBPS) {
}