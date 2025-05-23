// TelemBuilder.hpp
#ifndef __TELEM_BUILDER_H__
#define __TELEM_BUILDER_H__

#include <functional>
#include <set>
#include <string>
#include <vector>

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

/// @brief An internal struct for parsing global options
/// denoted with the `!!` prefix in telem
struct __OptionDescriptor {
    const char* name;
    OptionType type;
    std::function<void(TelemetryOptions&, const TokenData&)> apply;
};

/// @brief An internal struct for parsing message fields
/// denoted with the `>>` prefix in telem
struct __MessageFieldDescriptor {
    OptionType type;
    bool optional;
    std::function<void(CANMessageDescription&, const TokenData&)> apply;
};

/// @brief An internal struct for parsing signal fields
/// denoted with the `>>>` struct in telem
struct __SignalFieldDescriptor {
    OptionType type;
    bool optional;
    std::function<void(CANSignalDescription&, const TokenData&)> apply;
};

/// @brief A builder class for constructing a `CANBus` dbc from a .telem file
class TelemBuilder {
   public:
    /// @brief Base Ctor
    /// @param tokenizer The source of tokens from a telem config file
    explicit TelemBuilder(Tokenizer& tokenizer) : _tokenizer(tokenizer) {}

    /// @brief Builds a CAN bus DBC from the injected tokenizer
    /// @param bus The bus to add messages to, warning if it fails midway, only some mesages will be
    /// added
    /// @return A result of Telemetry Options, which are the global settings provided from the file
    Result<TelemetryOptions> build(CANBus& bus);

   private:
    Tokenizer& _tokenizer;

    static const __OptionDescriptor _optionTable[];
    static const __MessageFieldDescriptor _messageFieldTable[];
    static const __SignalFieldDescriptor _signalFieldTable[];

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
