#include "sd_manager.hpp"

#include "FS.h"
#include "SD.h"
#include "ff.h"
#include "sd_diskio.h"
#include "telemetry_debug.hpp"
#include "vfs_api.h"

namespace remote {

SDManager::SDManager(SDManagerConfig config)
    : _sd(FSImplPtr(new VFSImpl())), _config(config), _managerStatus(SD_BAD), _fileStackPtr(0) {}

void SDManager::initialize() {
    // attempt to mount the card (chip‐select first, then SPI)
    TELEM_DEBUG_PRINTLN("Initializing SD Card!");
    pinMode(_config.csPin, OUTPUT);
    if (_sd.begin(_config.csPin, _config.spi)) {
        _managerStatus = SD_GOOD;
    } else {
        TELEM_DEBUG_PRINT_ERRORLN("Unable to initialize SD Card!");
    }
}

common::Option<fs::File> SDManager::open(const char* path, const char* mode, const bool create) {
    if (_managerStatus == SD_BAD) {
        TELEM_DEBUG_PRINT_ERRORLN("Unable to open file %s! SDManager in a bad state!", path);
        return common::Option<fs::File>::none();
    }

    if (_fileStackPtr > 0) {
        fs::File current = _fileStack[_fileStackPtr - 1];
        if (strcmp(current.name(), path)) {
            // this is the same file
            TELEM_DEBUG_PRINTLN("Reusing the same file!");
            // check if we need to reopen
            if (current.availableForWrite() != 0 || current.available() != 0) {
                // no need to reopen the guy
                TELEM_DEBUG_PRINT_ERRORLN("No need to reopen!");
                return common::Option<fs::File>::some(current);
            } else {
                // uhh we need to reopen this guy
                _fileStackPtr--;
            }
            // otherwise we need to carry on with the rest of the logic
        }
    }

    // optionally create the file if it doesn't exist
    if (create && !_sd.exists(path)) {
        fs::File tmp = _sd.open(path, FILE_WRITE);
        if (!tmp) {
            _managerStatus = SD_BAD;
            TELEM_DEBUG_PRINT_ERRORLN("Unable to create file %s!", path);
            return common::Option<fs::File>::none();
        }
        tmp.close();
    }

    // make sure we don't overflow our stack
    if (_fileStackPtr >= MAX_STACK_SIZE) {
        _managerStatus = SD_BAD;
        TELEM_DEBUG_PRINT_ERRORLN("Opening file %s would result in stack overflow!", path);
        return common::Option<fs::File>::none();
    }

    // actually open in the requested mode
    fs::File f = _sd.open(path, mode);
    if (!f) {
        _managerStatus = SD_BAD;
        TELEM_DEBUG_PRINTLN("Unable to open file %s", path);
        return common::Option<fs::File>::none();
    }

    // push it onto our internal stack
    _fileStack[_fileStackPtr] = f;
    TELEM_DEBUG_PRINTLN("Opened file %s, stack ptr %d", path, _fileStackPtr);
    fs::File result = _fileStack[_fileStackPtr];
    _fileStackPtr++;
    return common::Option<fs::File>::some(result);
}

void SDManager::flush() {
    TELEM_DEBUG_PRINTLN("Flushing file %d", _fileStackPtr - 1);
    if (_fileStackPtr > 0) {
        fs::File file = _fileStack[_fileStackPtr - 1];
        TELEM_DEBUG_PRINTLN("Flushed file %s", file.name());
        file.flush();
    }
}

void SDManager::close() {
    TELEM_DEBUG_PRINTLN("Closing file %d", _fileStackPtr - 1);
    if (_fileStackPtr > 0) {
        _fileStackPtr--;
        fs::File file = _fileStack[_fileStackPtr];
        TELEM_DEBUG_PRINTLN("Closed file %s", file.name());
        file.close();
    }
}

FileGuard::FileGuard(SDManager& manager, const char* path, const char* mode,
                     FileGaurdBehavior behavior, bool create)
    : _manager(manager), _status(FILE_BAD), _behavior(behavior) {
    // Ask the manager to open for us:
    TELEM_DEBUG_PRINTLN("File Guard ctor");
    auto optFile = manager.open(path, mode, create);
    if (optFile.isSome()) {
        _file = optFile.value();
        _status = FILE_GOOD;
    }
}

FileGuard::~FileGuard() {
    TELEM_DEBUG_PRINTLN("File Guard dtor");
    if (_status == FILE_GOOD) {
        if (_behavior == FGB_FLUSH_ON_DESTRUCTION) {
            _manager.flush();
        } else {
            _manager.close();
        }
    }
}

common::Option<fs::File> FileGuard::file() {
    if (_status == FILE_GOOD) {
        // Return a copy of the handle to the user
        return common::Option<fs::File>::some(_file);
    }
    return common::Option<fs::File>::none();
}

void SDManager::createDir(const char* dir) {
    if (_managerStatus != SD_GOOD) {
        TELEM_DEBUG_PRINT_ERRORLN("Unable to create dir %s! SDManager in a bad state!", dir);
        return;
    }

    // If it already exists, do nothing
    if (!_sd.exists(dir)) {
        if (!_sd.mkdir(dir)) {
            _managerStatus = SD_BAD;
        }
    }
}

uint16_t SDManager::numFilesInDir(const char* dir) {
    if (_managerStatus != SD_GOOD) {
        TELEM_DEBUG_PRINT_ERRORLN("Unable to count files in dir %s! SDManager in a bad state!",
                                  dir);
        return 0;
    }

    uint16_t count = 0;
    fs::File d = _sd.open(dir);
    if (!d || !d.isDirectory()) {
        return 0;
    }

    // iterate through all entries
    fs::File entry = d.openNextFile();
    while (entry) {
        ++count;
        entry.close();
        entry = d.openNextFile();
    }
    d.close();
    return count;
}

}  // namespace remote
