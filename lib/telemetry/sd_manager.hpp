#ifndef __SD_MANAGER_H__
#define __SD_MANAGER_H__

#include <SD.h>

#include <array>
#include <define.hpp>
#include <map>
#include <option.hpp>

namespace remote {

struct SDManagerConfig {
    SPIClass& spi;
    HWPin csPin;
};

enum SDManagerStatus { SD_GOOD, SD_BAD };

enum FileStatus { FILE_GOOD, FILE_BAD };

enum FileGaurdBehavior { FGB_CLOSE_ON_DESTRUCTION, FGB_FLUSH_ON_DESTRUCTION };

class FileGuard;

class SDManager {
   public:
    SDManager(SDManagerConfig config);
    void initialize();
    friend class FileGuard;

   private:
    static constexpr std::size_t MAX_STACK_SIZE = 8;

    fs::SDFS _sd;
    fs::File _openFile;
    SDManagerConfig _config;
    SDManagerStatus _managerStatus;

    std::array<fs::File, MAX_STACK_SIZE> _fileStack;
    std::size_t _fileStackPtr;

    common::Option<fs::File> open(const char* path, const char* mode, const bool create = false);
    void flush();
    void close();
};

class FileGuard {
   public:
    FileGuard(SDManager& manager, const char* path, const char* mode, FileGaurdBehavior behavior,
              bool create = false);
    ~FileGuard();

    common::Option<fs::File> file();

   private:
    FileStatus _status;
    FileGaurdBehavior _behavior;
    fs::File _file;
};

}  // namespace remote

#endif  // __SD_MANAGER_H__