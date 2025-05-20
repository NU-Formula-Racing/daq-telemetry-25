#include "sd_manager.hpp"

#include "vfs_api.h"
#include "sd_diskio.h"
#include "ff.h"
#include "FS.h"
#include "SD.h"

namespace remote {


SDManager::SDManager(SDManagerConfig config)
    : _sd(FSImplPtr(new VFSImpl())), _config(config), _managerStatus(SD_BAD), _fileStackPtr(0) {}

void SDManager::initialize() {
    // attempt to mount the card (chip‐select first, then SPI)
    if (_sd.begin(_config.csPin, _config.spi)) {
        _managerStatus = SD_GOOD;
    }
}

common::Option<fs::File> SDManager::open(const char* path, const char* mode, const bool create) {
    if (_managerStatus == SD_BAD) {
        return common::Option<fs::File>::none();
    }

    // optionally create the file if it doesn't exist
    if (create && !_sd.exists(path)) {
        fs::File tmp = _sd.open(path, FILE_WRITE);
        if (!tmp) {
            _managerStatus = SD_BAD;
            return common::Option<fs::File>::none();
        }
        tmp.close();
    }

    // make sure we don't overflow our stack
    if (_fileStackPtr >= MAX_STACK_SIZE) {
        _managerStatus = SD_BAD;
        return common::Option<fs::File>::none();
    }

    // actually open in the requested mode
    fs::File f = _sd.open(path, mode);
    if (!f) {
        _managerStatus = SD_BAD;
        return common::Option<fs::File>::none();
    }

    // push it onto our internal stack
    _fileStack[_fileStackPtr++] = std::move(f);

    // pop it right back off, and hand it back to the caller
    fs::File result = std::move(_fileStack[--_fileStackPtr]);
    return common::Option<fs::File>::some(std::move(result));
}

void SDManager::flush() {
    if (_fileStackPtr > 0) {
        _fileStack[_fileStackPtr - 1].flush();
    }
}

void SDManager::close() {
    if (_fileStackPtr > 0) {
        _fileStack[--_fileStackPtr].close();
    }
}


FileGuard::FileGuard(SDManager& manager, const char* path, const char* mode,
                     FileGaurdBehavior behavior, bool create)
    : _status(FILE_BAD), _behavior(behavior) {
    // Ask the manager to open for us:
    auto optFile = manager.open(path, mode, create);
    if (optFile) {
        _file = std::move(optFile.value());
        _status = FILE_GOOD;
    }
}

FileGuard::~FileGuard() {
    if (_status == FILE_GOOD) {
        if (_behavior == FGB_FLUSH_ON_DESTRUCTION) {
            _file.flush();
        } else {
            _file.close();
        }
    }
}

common::Option<fs::File> FileGuard::file() {
    if (_status == FILE_GOOD) {
        // Return a copy of the handle to the user
        return common::Option<fs::File>::some(std::move(_file));
    }
    return common::Option<fs::File>::none();
}

}  // namespace remote
