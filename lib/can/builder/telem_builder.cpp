#include "telem_builder.hpp"

#include <cstring>

#include "can.hpp"
#include "option.hpp"
#include "result.hpp"
#include "tokenizer.hpp"

namespace can {

using common::Option;
using common::Result;

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
    TelemetryOptions opts;

    if (!_tokenizer.start()) {
        return Result<TelemetryOptions>::errorResult("failed to start tokenizer");
    }

    // Phase 1: global options
    while (true) {
        Option<Token> p = _tokenizer.peek();
        if (!p.isSome() || p.value().type != TokenType::TT_OPTION_PREFIX) {
            break;
        }
        Result<bool> r = _parseGlobalOption(opts);
        if (r.isError()) {
            _tokenizer.end();
            return Result<TelemetryOptions>::errorResult(r.error());
        }
    }

    // Phase 2: boards
    bool sawBoard = false;
    while (true) {
        Option<Token> p = _tokenizer.peek();
        if (!p.isSome()) {
            break;  // EOF
        }
        Token t = p.value();
        if (t.type == TokenType::TT_BOARD_PREFIX) {
            sawBoard = true;
            Result<bool> br = _parseBoard(bus);
            if (br.isError()) {
                _tokenizer.end();
                return Result<TelemetryOptions>::errorResult(br.error());
            }
        } else {
            // any other prefix at top-level is invalid
            _tokenizer.end();
            return Result<TelemetryOptions>::errorResult(
                "unexpected token at top level; expected '>' or EOF");
        }
    }

    if (!sawBoard) {
        _tokenizer.end();
        return Result<TelemetryOptions>::errorResult("no board defined");
    }

    _tokenizer.end();
    return Result<TelemetryOptions>::ok(opts);
}

Result<bool> TelemBuilder::_parseGlobalOption(TelemetryOptions& opts) {
    _tokenizer.next();  // consume '!!'

    Option<Token> nameTok = _tokenizer.next();
    Option<Token> valTok = _tokenizer.next();
    if (!nameTok.isSome() || nameTok.value().type != TokenType::TT_IDENTIFIER || !valTok.isSome() ||
        valTok.value().type != TokenType::TT_INT) {
        return Result<bool>::errorResult("malformed global option");
    }

    const char* name = IdentifierPool::instance().get(nameTok.value().data.idHandle);

    return _applyOptionByName(opts, std::string(name), valTok.value().data);
}

Result<bool> TelemBuilder::_applyOptionByName(TelemetryOptions& opts, const std::string& name,
                                              const TokenData& data) {
    for (std::size_t i = 0; i < sizeof(_optionTable) / sizeof(_optionTable[0]); ++i) {
        const __OptionDescriptor& od = _optionTable[i];
        if (name == od.name) {
            od.apply(opts, data);
            return Result<bool>::ok(true);
        }
    }
    return Result<bool>::errorResult(std::string("unknown option: ") + name);
}

Result<bool> TelemBuilder::_parseBoard(CANBus& bus) {
    // consume '>'
    _tokenizer.next();

    // board name
    Option<Token> nm = _tokenizer.next();
    if (!nm.isSome() || nm.value().type != TokenType::TT_IDENTIFIER) {
        return Result<bool>::errorResult("expected board name after '>'");
    }

    // at least one message
    bool hasMsg = false;
    while (true) {
        Option<Token> p = _tokenizer.peek();
        if (!p.isSome() || p.value().type != TokenType::TT_MESSAGE_PREFIX) {
            break;
        }
        hasMsg = true;
        // consume '>>'
        _tokenizer.next();

        Result<CANMessageDescription> mr = _parseMessage();
        if (mr.isError()) {
            return Result<bool>::errorResult(mr.error());
        }
        CANMessageDescription desc = mr.value();

        Result<bool> vr = _validateMessage(desc);
        if (vr.isError()) {
            return vr;
        }

        bus.addMessage(desc);
    }

    if (!hasMsg) {
        return Result<bool>::errorResult("board without any messages");
    }
    return Result<bool>::ok(true);
}

Result<CANMessageDescription> TelemBuilder::_parseMessage() {
    CANMessageDescription msgDesc{};
    msgDesc.signals.clear();

    // consume message name
    Option<Token> n = _tokenizer.next();
    if (!n.isSome() || n.value().type != TokenType::TT_IDENTIFIER) {
        return Result<CANMessageDescription>::errorResult("expected message name");
    }


    // header fields via LUT
    for (std::size_t i = 0; i < sizeof(_messageFieldTable) / sizeof(_messageFieldTable[0]); ++i) {
        Option<Token> ft = _tokenizer.next();
        if (!ft.isSome()) {
            return Result<CANMessageDescription>::errorResult("incomplete message header");
        }
        Token tok = ft.value();
        // type‐check
        if (_messageFieldTable[i].type == OptionType::UINT32 && tok.type != TokenType::TT_HEX_INT) {
            return Result<CANMessageDescription>::errorResult("expected hex ID in message header");
        }
        if (_messageFieldTable[i].type == OptionType::UINT16 && tok.type != TokenType::TT_INT) {
            return Result<CANMessageDescription>::errorResult(
                "expected integer size in message header");
        }
        _messageFieldTable[i].apply(msgDesc, tok.data);
    }
    msgDesc.type = STANDARD;

    // signals
    std::size_t msgBits = static_cast<std::size_t>(msgDesc.length) * 8;
    bool sawSig = false;
    while (true) {
        Option<Token> p = _tokenizer.peek();
        if (!p.isSome() || p.value().type != TokenType::TT_SIGNAL_PREFIX) {
            break;
        }
        sawSig = true;
        // consume '>>>'
        _tokenizer.next();

        Result<CANSignalDescription> sr = _parseSignal();
        if (sr.isError()) {
            return Result<CANMessageDescription>::errorResult(sr.error());
        }
        CANSignalDescription sig = sr.value();

        Result<bool> vr = _validateSignal(sig, msgBits);
        if (vr.isError()) {
            return Result<CANMessageDescription>::errorResult(vr.error());
        }

        msgDesc.signals.push_back(sig);
    }

    if (!sawSig) {
        return Result<CANMessageDescription>::errorResult("message without any signals");
    }
    return Result<CANMessageDescription>::ok(msgDesc);
}

Result<bool> TelemBuilder::_validateMessage(const CANMessageDescription& msg) {
    if (msg.id > 0x7FF) {
        return Result<bool>::errorResult("message ID out of 0x000–0x7FF");
    }
    // additional checks (duplicate names, etc.) go here
    return Result<bool>::ok(true);
}

Result<CANSignalDescription> TelemBuilder::_parseSignal() {
    // name
    Option<Token> n = _tokenizer.next();
    if (!n.isSome() || n.value().type != TokenType::TT_IDENTIFIER) {
        return Result<CANSignalDescription>::errorResult("expected signal name");
    }
    // dataType
    Option<Token> dt = _tokenizer.next();
    if (!dt.isSome() || dt.value().type != TokenType::TT_IDENTIFIER) {
        return Result<CANSignalDescription>::errorResult("expected signal dataType");
    }
    const char* dtype = IdentifierPool::instance().get(dt.value().data.idHandle);

    CANSignalDescription sigDesc{};
    // numeric fields via LUT
    for (std::size_t i = 0; i < sizeof(_signalFieldTable) / sizeof(_signalFieldTable[0]); ++i) {
        Option<Token> ft = _tokenizer.next();
        if (!ft.isSome()) {
            return Result<CANSignalDescription>::errorResult("incomplete signal definition");
        }
        Token tok = ft.value();
        // type‐check
        if (_signalFieldTable[i].type == OptionType::UINT16 && tok.type != TokenType::TT_INT) {
            return Result<CANSignalDescription>::errorResult("expected integer in signal field");
        }
        if (_signalFieldTable[i].type == OptionType::DOUBLE &&
            !(tok.type == TokenType::TT_FLOAT || tok.type == TokenType::TT_INT)) {
            return Result<CANSignalDescription>::errorResult("expected numeric in signal field");
        }
        _signalFieldTable[i].apply(sigDesc, tok.data);
    }

    // signedness override
    Option<Token> p1 = _tokenizer.peek();
    if (p1.isSome() && p1.value().type == TokenType::TT_IDENTIFIER) {
        const char* s = IdentifierPool::instance().get(p1.value().data.idHandle);
        if (std::strcmp(s, "signed") == 0 || std::strcmp(s, "unsigned") == 0) {
            sigDesc.isSigned = (std::strcmp(s, "signed") == 0);
            _tokenizer.next();
        }
    }

    // endianness override
    Option<Token> p2 = _tokenizer.peek();
    if (p2.isSome() && p2.value().type == TokenType::TT_IDENTIFIER) {
        const char* e = IdentifierPool::instance().get(p2.value().data.idHandle);
        if (std::strcmp(e, "big") == 0 || std::strcmp(e, "little") == 0) {
            sigDesc.endianness = (std::strcmp(e, "big") == 0 ? MSG_BIG_ENDIAN : MSG_LITTLE_ENDIAN);
            _tokenizer.next();
        }
    }

    return Result<CANSignalDescription>::ok(sigDesc);
}

// ─── Validate signal constraints (bit-fit, overlaps) ────────────────────────
Result<bool> TelemBuilder::_validateSignal(const CANSignalDescription& sig, size_t msgBits) {
    if ((size_t)sig.startBit + (size_t)sig.length > msgBits) {
        return Result<bool>::errorResult("signal overruns message payload");
    }
    // overlap checks would go here...
    return Result<bool>::ok(true);
}

}  // namespace can
