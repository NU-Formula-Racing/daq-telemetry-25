#include "telem_builder.hpp"

#include <cstring>
#include <vector>

#include "can.hpp"
#include "can_debug.hpp"
#include "option.hpp"
#include "result.hpp"
#include "token_reader.hpp"
#include "tokenizer.hpp"

namespace can {

using common::Option;
using common::Result;

// Global options
const __OptionDescriptor TelemBuilder::_optionTable[] = {
    {.name = "logPeriodMs",
     .type = OptionType::UINT16,
     .apply = [](TelemetryOptions& o,
                 const TokenData& d) { o.logPeriodMs = static_cast<uint16_t>(d.intValue); }},
    {.name = "wirelessPeriodMs",
     .type = OptionType::UINT16,
     .apply = [](TelemetryOptions& o, const TokenData& d) {
         o.wirelessPeriodMs = static_cast<uint16_t>(d.intValue);
     }}};

// Fields in a message header: ID, size
const __MessageFieldDescriptor TelemBuilder::_messageFieldTable[] = {
    {.type = OptionType::UINT32,
     .optional = false,
     .apply = [](CANMessageDescription& m,
                 const TokenData& d) { m.id = static_cast<uint32_t>(d.uintValue); }},
    {.type = OptionType::UINT16,
     .optional = false,
     .apply = [](CANMessageDescription& m, const TokenData& d) {
         m.length = static_cast<uint8_t>(d.intValue);
     }}};

// Fields in a signal line: startBit, length, factor, offset
const __SignalFieldDescriptor TelemBuilder::_signalFieldTable[] = {
    {.type = OptionType::UINT16,
     .optional = false,
     .apply = [](CANSignalDescription& s,
                 const TokenData& d) { s.startBit = static_cast<uint8_t>(d.intValue); }},
    {.type = OptionType::UINT16,
     .optional = false,
     .apply = [](CANSignalDescription& s,
                 const TokenData& d) { s.length = static_cast<uint8_t>(d.intValue); }},
    {.type = OptionType::DOUBLE,
     .optional = false,
     .apply = [](CANSignalDescription& s, const TokenData& d) { s.factor = d.floatValue; }},
    {.type = OptionType::DOUBLE,
     .optional = false,
     .apply = [](CANSignalDescription& s, const TokenData& d) { s.offset = d.floatValue; }}};

Result<TelemetryOptions> TelemBuilder::build(CANBus& bus) {
    TelemetryOptions options;
    if (!_tokenizer.start()) {
        return Result<TelemetryOptions>::errorResult("Failed to start tokenizer");
    }

    // Phase 1: parse global options
    while (true) {
        Option<Token> peekToken = _tokenizer.peek();
        if (!peekToken.isSome() || peekToken.value().type != TokenType::TT_OPTION_PREFIX) {
            break;
        }
        Result<bool> optRes = _parseGlobalOption(options);
        if (optRes.isError()) {
            _tokenizer.end();
            return Result<TelemetryOptions>::errorResult(optRes.error());
        }
    }

    // Phase 2: parse boards and their messages
    while (true) {
        Option<Token> peekToken = _tokenizer.peek();
        if (!peekToken.isSome()) {
            break;  // EOF
        }
        Token token = peekToken.value();

        if (token.type == TokenType::TT_BOARD_PREFIX) {
            // consume '>'
            _tokenizer.next();
            // consume board name
            Option<Token> boardNameToken = _tokenizer.next();
            if (!boardNameToken.isSome() ||
                boardNameToken.value().type != TokenType::TT_IDENTIFIER) {
                _tokenizer.end();
                return Result<TelemetryOptions>::errorResult("Expected board name after '>'");
            }

            // parse messages under this board
            while (true) {
                Option<Token> nextToken = _tokenizer.peek();
                if (!nextToken.isSome() || nextToken.value().type != TokenType::TT_MESSAGE_PREFIX) {
                    break;
                }
                // consume '>>'
                _tokenizer.next();

                Result<CANMessageDescription> msgRes = _parseMessage();
                if (msgRes.isError()) {
                    _tokenizer.end();
                    return Result<TelemetryOptions>::errorResult(msgRes.error());
                }
                CANMessageDescription msgDesc = msgRes.value();
                bus.addMessage(msgDesc);
            }
        } else {
            // skip anything else
            _tokenizer.next();
        }
    }

    _tokenizer.end();
    return Result<TelemetryOptions>::ok(options);
}

Result<bool> TelemBuilder::_parseGlobalOption(TelemetryOptions& opts) {
    // consume '!!'
    _tokenizer.next();
    Option<Token> nameToken = _tokenizer.next();
    Option<Token> valueToken = _tokenizer.next();

    if (!nameToken.isSome() || nameToken.value().type != TokenType::TT_IDENTIFIER ||
        !valueToken.isSome() || valueToken.value().type != TokenType::TT_INT) {
        return Result<bool>::errorResult("Malformed global option");
    }
    const char* name = IdentifierPool::instance().get(nameToken.value().data.idHandle);
    Result<bool> applyRes = _applyOptionByName(opts, name, valueToken.value().data);
    return applyRes;
}

Result<CANMessageDescription> TelemBuilder::_parseMessage() {
    // consume message name
    Option<Token> nameToken = _tokenizer.next();
    CANMessageDescription message{};

    // header fields
    for (std::size_t i = 0; i < sizeof(_messageFieldTable) / sizeof(_messageFieldTable[0]); ++i) {
        Option<Token> fieldToken = _tokenizer.next();
        if (!fieldToken.isSome()) {
            return Result<CANMessageDescription>::errorResult("Incomplete message header");
        }
        TokenData data = fieldToken.value().data;
        if (fieldToken.value().type == TokenType::TT_HEX_INT) {
            data.uintValue = fieldToken.value().data.uintValue;
        }
        _messageFieldTable[i].apply(message, data);
    }
    message.type = STANDARD;

    // parse signals
    while (true) {
        Option<Token> nextTok = _tokenizer.peek();
        if (!nextTok.isSome() || nextTok.value().type != TokenType::TT_SIGNAL_PREFIX) {
            // break out of this!
            return Result<CANMessageDescription>::ok(message);
        }

        _tokenizer.next();  // consume '>>>'
        Result<CANSignalDescription> sigRes = _parseSignal();
        if (sigRes.isError()) {
            return Result<CANMessageDescription>::errorResult(sigRes.error());
        }
        message.signals.push_back(sigRes.value());
    }

    return Result<CANMessageDescription>::ok(message);
}

Result<CANSignalDescription> TelemBuilder::_parseSignal() {
    // consume name and type
    _tokenizer.next();
    _tokenizer.next();
    CANSignalDescription signal{};

    for (std::size_t i = 0; i < sizeof(_signalFieldTable) / sizeof(_signalFieldTable[0]); ++i) {
        Option<Token> fieldToken = _tokenizer.next();
        if (!fieldToken.isSome()) {
            return Result<CANSignalDescription>::errorResult("Incomplete signal definition");
        }
        TokenData data = fieldToken.value().data;
        if (fieldToken.value().type == TokenType::TT_FLOAT) {
            data.floatValue = fieldToken.value().data.floatValue;
        }
        _signalFieldTable[i].apply(signal, data);
    }

    return Result<CANSignalDescription>::ok(signal);
}

Result<bool> TelemBuilder::_applyOptionByName(TelemetryOptions& opts, const std::string& name,
                                              const TokenData& data) {
    for (std::size_t i = 0; i < sizeof(_optionTable) / sizeof(_optionTable[0]); ++i) {
        const __OptionDescriptor& desc = _optionTable[i];
        if (name == desc.name) {
            desc.apply(opts, data);
            return Result<bool>::ok(true);
        }
    }
    return Result<bool>::errorResult(std::string("Unknown option: ") + name);
}

}  // namespace can
