#ifndef __SD_TOKEN_READER_H__
#define __SD_TOKEN_READER_H__

#include <builder/token_reader.hpp>
#include <cstddef>
#include <cstdint>
#include <string>
#include <sd_manager.hpp>

namespace remote {

/// @brief SD-backed reader; slurps file on start()
class SDTokenReader : public can::TokenReader {
   public:
    explicit SDTokenReader(const std::string& filename);

    bool start() override;
    bool peekNextWord(std::size_t maxLength, char* charBuf, std::size_t* length) override;
    bool moveWord(std::size_t stepSize = 1) override;
    bool eatUntil(const char character) override;
    void end() override;

   private:
    std::string _filename;
    FileGuard _guard;
};

}  // namespace remote

#endif  // __SD_TOKEN_READER_H__