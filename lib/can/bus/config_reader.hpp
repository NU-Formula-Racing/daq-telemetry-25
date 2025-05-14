#ifndef __CONFIG_READER_H__
#define __CONFIG_READER_H__

#include <stddef.h>
#include <stdint.h>

#include <string>

namespace can {

/// @brief An abstract class for reading a configuration file, while minimizing memory footprint
/// Why? We don't want to read the entire file at once -- we would need to allocate a full string to read it
/// This has historically caused problems with larger files in NFR24's wirless implementation
/// This class is abstract for testability
class ConfigReader {
   public:
    /// @brief Begin reading
    /// @return If the reading was sucessful
    virtual bool start() { return false; }

    /// @brief Get the next word in the file, delimited by a space
    /// @param maxLength The size of charBuf
    /// @param charBuf The location to place the c-style string (memory location provided by user)
    /// @param length The length of the c-style string
    /// @return True if peeking was sucessful (there was a word), otherwise false
    virtual bool peekNextWord(size_t maxLength, const char *charBuf, size_t *length) { return false; }

    /// @brief Move what word we are in the config file
    /// @param stepSize The step size (how many words to move? Negative means go back). Default is 1.
    /// @return True if you are able to move that amount of words
    virtual bool moveWord(size_t stepSize = 1) { return false; }

    /// @brief End the reading
    virtual void end() {}
};

/// @brief A mock implementation of ConfigReader, where you simply give the full contents of the file
/// Meant for use in native testing, but not in embded
class MockConfigReader : public ConfigReader {
   public:
    MockConfigReader(std::string configContents) : _content(configContents) {}

    virtual bool start();
    virtual bool peekNextWord(size_t maxLength, const char *charBuf, size_t *length);
    virtual bool moveWord(size_t stepSize = 1);

   private:
    std::string _content;
};

#ifdef __PLATFORM_ESP32

/// @brief A mock implementation of ConfigReader, where you simply give the full contents of the file
/// Meant for use in native testing, but not in embded
class SDConfigReader : public ConfigReader {
   public:
    SDConfigReader(std::string filename) : _filename(filename) {}

    virtual bool start();
    virtual bool peekNextWord(size_t maxLength, const char *charBuf, size_t *length);
    virtual bool moveWord(size_t stepSize = 1);

   private:
    std::string _filename;
};

#endif  // __PLATFORM_ESP32

}  // namespace can

#endif  // __CONFIG_READER_H__