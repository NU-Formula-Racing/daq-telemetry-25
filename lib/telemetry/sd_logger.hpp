#ifndef __SD_LOGGER_H__
#define __SD_LOGGER_H__

#include <RTCLib.h>
#include <SD.h>

#include <can.hpp>
#include <option.hpp>
#include <sd_manager.hpp>
#include <sstream>

#include "remote_debug.hpp"

namespace remote {

class SDLogger {
   public:
    SDLogger(SDManager& manager, RTC_PCF8523& rtc) : _manager(manager), _rtc(rtc) {}

    void initialize() {
        // create a file name from the date
        DateTime time = _rtc.now();
        String dateTime = time.timestamp(DateTime::TIMESTAMP_DATE);

        std::stringstream ss;
        ss << "/" << dateTime.c_str();
        _dir = ss.str();

        REMOTE_DEBUG_PRINTLN("Creating directory %s", _dir.c_str());
        _manager.createDir(_dir.c_str());

        int numFiles = _manager.numFilesInDir(_dir.c_str());

        ss << "/log_" << numFiles << ".daq";
        _filename = ss.str();
        REMOTE_DEBUG_PRINTLN("Logging to file %s", _filename.c_str());
        // open the file
        FileGuard guard(_manager, _filename.c_str(), FILE_WRITE, FGB_CLOSE_ON_DESTRUCTION, true);
    }

    void log(const can::CANBus &bus) {
        FileGuard guard(_manager, _filename.c_str(), FILE_WRITE, FGB_FLUSH_ON_DESTRUCTION, true);

        common::Option<fs::File> fileOpt = guard.file();

        if (fileOpt.isNone()) {
            REMOTE_DEBUG_PRINT_ERRORLN("Unable to log! File can't be opened!");
            return;
        }

        fs::File file = fileOpt.value();

        // go to the end of the file
        std::size_t size;
        const uint8_t *buffer = bus.dataBuffer(&size);
        REMOTE_DEBUG_PRINTLN("Writing %d bytes!", size);
        file.write(buffer, size);
    }

   private:
    std::string _dir;
    std::string _filename;
    SDManager &_manager;
    RTC_PCF8523& _rtc;
};

}  // namespace remote

#endif  // __SD_LOGGER_H__