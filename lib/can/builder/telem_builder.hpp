#ifndef __TELEM_BUILDER_H__
#define __TELEM_BUILDER_H__

#include <functional>
#include <string>
#include <vector>

#include "can.hpp"
#include "can_debug.hpp"
#include "option.hpp"
#include "result.hpp"
#include "token_reader.hpp"
#include "tokenizer.hpp"

namespace can {

/// @brief All of the global telemetry options we support
struct TelemetryOptions {
    uint16_t logPeriodMs = 50;
    uint16_t wirelessPeriodMs = 100;
};

/// @brief What kind of data an option holds
enum class OptionType { UINT16, UINT32, FLOAT, DOUBLE, BOOL };

/// @brief Describes one global option: its name, type, and how to apply it
struct __OptionDescriptor {
    const char* name;
    OptionType type;
    std::function<void(TelemetryOptions&, const TokenData&)> apply;
};

/// @brief Describes how to parse one field from a message header
struct __MessageFieldDescriptor {
    OptionType type;
    bool optional;
    std::function<void(CANMessageDescription&, const TokenData&)> apply;
};

/// @brief Describes how to parse one field from a signal line
struct __SignalFieldDescriptor {
    OptionType type;
    bool optional;
    std::function<void(CANSignalDescription&, const TokenData&)> apply;
};

class TelemBuilder {
   public:
    explicit TelemBuilder(Tokenizer& tokenizer) : _tokenizer(tokenizer) {}

    /// Parse the entire DBC-like stream into CANBus messages and global options
    Result<TelemetryOptions> build(CANBus& bus);

   private:
    Tokenizer& _tokenizer;

    static const __OptionDescriptor _optionTable[];
    static const __MessageFieldDescriptor _messageFieldTable[];
    static const __SignalFieldDescriptor _signalFieldTable[];

    // ------ helper parsers ------
    Result<bool> _parseGlobalOption(TelemetryOptions& opts);
    Result<CANMessageDescription> _parseMessage();
    Result<CANSignalDescription> _parseSignal();
    Result<bool> _skipEnumEntries();
    Result<bool> _applyOptionByName(TelemetryOptions& opts, const std::string& name,
                                    const TokenData& data);
};

}  // namespace can

#endif  // __TELEM_BUILDER_H__