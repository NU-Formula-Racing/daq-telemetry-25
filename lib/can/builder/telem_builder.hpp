// TelemBuilder.hpp
#ifndef __TELEM_BUILDER_H__
#define __TELEM_BUILDER_H__

#include <functional>
#include <string>
#include <vector>
#include <set>

#include "can.hpp"
#include "option.hpp"
#include "result.hpp"
#include "tokenizer.hpp"

namespace can {

/// @brief All of the global telemetry options we support
struct TelemetryOptions {
    uint16_t logPeriodMs = 50;
    uint16_t wirelessPeriodMs = 100;
};

enum class OptionType { UINT16, UINT32, FLOAT, DOUBLE, BOOL };

struct __OptionDescriptor {
    const char* name;
    OptionType type;
    std::function<void(TelemetryOptions&, const TokenData&)> apply;
};

struct __MessageFieldDescriptor {
    OptionType type;
    bool optional;
    std::function<void(CANMessageDescription&, const TokenData&)> apply;
};

struct __SignalFieldDescriptor {
    OptionType type;
    bool optional;
    std::function<void(CANSignalDescription&, const TokenData&)> apply;
};

class TelemBuilder {
   public:
    explicit TelemBuilder(Tokenizer& tokenizer) : _tokenizer(tokenizer) {}

    /// Parses the entire stream, registers messages on `bus`, and returns global options.
    Result<TelemetryOptions> build(CANBus& bus);

   private:
    Tokenizer& _tokenizer;

    static const __OptionDescriptor _optionTable[];
    static const __MessageFieldDescriptor _messageFieldTable[];
    static const __SignalFieldDescriptor _signalFieldTable[];

    // ——— Helpers ———
    Result<bool> _parseGlobalOption(TelemetryOptions& opts);
    Result<bool> _applyOptionByName(TelemetryOptions& opts, const std::string& name,
                                    const TokenData& data);
    Result<bool> _parseBoard(CANBus& bus);
    Result<CANMessageDescription> _parseMessage();
    Result<CANSignalDescription> _parseSignal();
    Result<bool> _validateMessage(const CANMessageDescription& msg);
    Result<bool> _validateSignal(const CANSignalDescription& sig, size_t msgBits);

    std::set<uint16_t> _messageIDSet;
};

}  // namespace can

#endif  // __TELEM_BUILDER_H__
