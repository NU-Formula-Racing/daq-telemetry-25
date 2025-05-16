#ifndef __TASK_LOG_H__
#define __TASK_LOG_H__

#include <RTCLib.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>

#include <define.hpp>
#include <iostream>
#include <tasks.hpp>

namespace remote {

class LogTask : public tasks::TaskAction {
   public:
    bool initialize() {
        Wire.begin(HWPin::LGR_RTC_SDA, HWPin::LGR_RTC_SCL, 100000);
        _rtc.begin();
        pinMode(HWPin::LOGGER_SD_CS, OUTPUT);

        if (!_rtc.initialized() || _rtc.lostPower()) {
            REMOTE_DEBUG_PRINTLN("RTC is NOT initialized, let's set the time!");
            _rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        }

        return true;
    }

    void run() {
        digitalWrite(HWPin::LOGGER_SD_CS, HIGH);
        if (_rtc.initialized()) {
            DateTime now = _rtc.now();
            REMOTE_DEBUG_PRINTLN("%s", now.timestamp().c_str());
        } else {
            REMOTE_DEBUG_PRINTLN("Not initalized!");
        }
        digitalWrite(HWPin::LOGGER_SD_CS, LOW);
    }

    void end() {}

   private:
    RTC_PCF8523 _rtc;
};

}  // namespace remote
#endif  // __TASK_LOG_H__
