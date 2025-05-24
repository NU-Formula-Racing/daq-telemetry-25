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

enum LoggerState { LOGGER_BAD, LOGGER_GOOD };

static std::vector<uint8_t> __HEADER = {'N', 'F', 'R', '2', '5', '1', '0', '0'};

class SDLogger {
   public:
    SDLogger(SDManager& manager, RTC_PCF8523& rtc)
        : _manager(manager), _rtc(rtc), _state(LoggerState::LOGGER_BAD) {}

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

        common::Option<fs::File> fileOpt = guard.file();

        if (fileOpt.isNone()) {
            REMOTE_DEBUG_PRINT_ERRORLN("Unable to log! File can't be opened!");
            _state = LOGGER_BAD;
            return;
        }

        _state = LOGGER_GOOD;

        // write the header
        fs::File file = fileOpt.value();

        file.write(__HEADER.data(), __HEADER.size());
    }

    void log(const can::CANBus& bus) {
        if (_state == LOGGER_BAD) {
            REMOTE_DEBUG_PRINT_ERRORLN("Unable to log! Logger state is bad!");
        }

        FileGuard guard(_manager, _filename.c_str(), FILE_WRITE, FGB_CLOSE_ON_DESTRUCTION, false);

        common::Option<fs::File> fileOpt = guard.file();

        if (fileOpt.isNone()) {
            REMOTE_DEBUG_PRINT_ERRORLN("Unable to log! File can't be opened!");
            _state = LOGGER_BAD;
            return;
        }

        fs::File file = fileOpt.value();

        // go to the end of the file
        file.seek(file.size());

        std::size_t size;
        const uint8_t* buffer = bus.dataBuffer(&size);
        std::size_t actualSize = file.write(buffer, size);
        REMOTE_DEBUG_PRINTLN("Attempted to write %d bytes. Wrote %d bytes.", size, actualSize);
        REMOTE_DEBUG_PRINTLN("File is %lld bytes!", file.size());
    }

   private:
    std::string _dir;
    std::string _filename;
    SDManager& _manager;
    RTC_PCF8523& _rtc;
    LoggerState _state;
};

}  // namespace remote

#endif  // __SD_LOGGER_H__