#ifndef __TOKEN_READER_H__
#define __TOKEN_READER_H__

#include <cstddef>
#include <cstdint>
#include <string>

namespace can {

/// @brief Abstract interface for streaming through a token file word-by-word.
class TokenReader {
   public:
    virtual ~TokenReader() = default;

    /// @brief Prepare for reading (e.g. open file or stash string into buffer)
    /// @returns true on success
    virtual bool start() = 0;

    /// @brief Copy the next whitespace-delimited word into user buffer (without advancing).
    /// @param maxLength size of charBuf
    /// @param charBuf   user-owned buffer to receive a null-terminated string
    /// @param length    out: actual word length (may exceed maxLength-1)
    /// @return true if a word was available
    virtual bool peekNextWord(std::size_t maxLength, char* charBuf, std::size_t* length) = 0;

    /// @brief Advance the “cursor” by stepSize words (default 1).
    /// @return true if the move succeeded (didn’t run out)
    virtual bool moveWord(std::size_t stepSize = 1) = 0;

    /// @brief Advance the cursor until you encounter a given character
    /// @param character The character to advance until
    /// @return if the move succeeded
    virtual bool eatUntil(const char character) = 0;

    /// @brief Finish/cleanup (e.g. close file)
    virtual void end() = 0;
};

/// @brief In-memory reader for unit tests
class MockTokenReader : public TokenReader {
   public:
    explicit MockTokenReader(const std::string& tokenContents);

    bool start() override;
    bool peekNextWord(std::size_t maxLength, char* charBuf, std::size_t* length) override;
    bool moveWord(std::size_t stepSize = 1) override;
    bool eatUntil(const char character) override;
    void end() override;

   private:
    std::string _content;
    std::size_t _pos = 0;
};

#ifdef __PLATFORM_ESP32

/// @brief SD-backed reader; slurps file on start()
class SDTokenReader : public TokenReader {
   public:
    explicit SDTokenReader(const std::string& filename);

    bool start() override;
    bool peekNextWord(std::size_t maxLength, char* charBuf, std::size_t* length) override;
    bool moveWord(std::size_t stepSize = 1) override;
    bool eatUntil(const char character) override;
    void end() override;

   private:
    std::string _filename;
    std::size_t _pos = 0;
};

#endif  // __PLATFORM_ESP32

}  // namespace can

#endif  // __TOKEN_READER_H__
