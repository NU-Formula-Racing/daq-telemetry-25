#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <string>

#include "can_debug.hpp"
#include "option.hpp"
#include "token_reader.hpp"

namespace can {

/// @brief Kinds of tokens in the telemetry configuration language.
enum TokenType {
    TT_OPTION_PREFIX,   // "!!" global option
    TT_BOARD_PREFIX,    // ">" board definition
    TT_MESSAGE_PREFIX,  // ">>" message definition
    TT_SIGNAL_PREFIX,   // ">>>" signal definition
    TT_ENUM_PREFIX,     // ">>>>" enum entry
    TT_IDENTIFIER,     // identifiers (names)
    TT_HEX_INT,         // hexadecimal literal (0x...)
    TT_INT,            // decimal integer literal
    TT_FLOAT,          // floating-point literal
    TT_EOF       // end of input
};

/// @brief Handle representing an interned identifier.
struct IdentifierPoolHandle {
    std::size_t index;  // index into the pool
};

/// @brief Token payload data.
struct TokenData {
    union {
        int64_t intValue;
        uint64_t uintValue;
        double floatValue;
        IdentifierPoolHandle idHandle;  // identifier handle
    };
};

/// @brief A single lexical token.
struct Token {
    TokenType type;  // token kind
    TokenData data;  // token-specific data
};

/// @brief Interns identifier strings to provide stable, pooled storage.
/// Uses a small heap allocation (deque of strings).
class IdentifierPool {
   public:
    static IdentifierPool& instance() {
        static IdentifierPool _idPool;  // pool for identifier texts
        return _idPool;
    }

    /// @brief Construct an empty identifier pool.
    IdentifierPool();

    /// @brief Adds a copy of src into the pool.
    /// @param src Null-terminated source string.
    /// @return Handle to the pooled string.
    IdentifierPoolHandle intern(const char* src);

    /// @brief Retrieve the string for a given handle.
    /// @param h Handle returned by intern().
    /// @return Null-terminated string.
    const char* get(IdentifierPoolHandle h) const;

   private:
    std::deque<std::string> _pool;  // stored identifier strings
};

/// @brief Tokenizer: turns whitespace-delimited words into Token objects.
class Tokenizer {
   public:
    /// @brief Construct a tokenizer.
    /// @param reader Underlying word-based reader (file or memory).
    Tokenizer(TokenReader& reader) : _reader(reader) {}

    /// @brief Initialize the underlying reader. Must be called before next().
    /// @return true on success, false on failure.
    bool start();

    /// @brief Clean up reader resources.
    void end();

    /// @brief Retrieve the next token, or none() at EOF.
    /// @return Some(Token) if available, none() otherwise.
    common::Option<Token> next();

   private:
    TokenReader& _reader;  // word reader
};

}  // namespace can

#endif  // __TOKENIZER_H__
