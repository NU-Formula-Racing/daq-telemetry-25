#include "resources.hpp"

#include <RTCLib.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <can.h>

#include <can_drivers.hpp>
#include <define.hpp>
#include <sd_manager.hpp>
#include <tasks.hpp>

#include "remote_debug.hpp"

using namespace remote;

static SPIClass vspi(VSPI);
static SPIClass hspi(HSPI);

static const SDManagerConfig __SD_CONFIG = {
    .spi = hspi,
    .csPin = HWPin::LOGGER_SD_CS,
};

Resources::Resources()
    : _driveDriver(),
      driveBus(_driveDriver, can::CANBaudRate::CBR_500KBPS),
      _dataDriver(),
      dataBus(_dataDriver, can::CANBaudRate::CBR_500KBPS),
      _sdManager(__SD_CONFIG) {
    // initialize sd card manager
    _sdManager.initialize();

    // initialize the RTC
    Wire.begin(HWPin::LGR_RTC_SDA, HWPin::LGR_RTC_SCL, 100000);
    rtc.begin();
    pinMode(HWPin::LOGGER_SD_CS, OUTPUT);

    if (!rtc.initialized() || rtc.lostPower()) {
        REMOTE_DEBUG_PRINTLN("RTC is NOT initialized, let's set the time!");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    // initialize the SD Logger
    logger.initialize();
}