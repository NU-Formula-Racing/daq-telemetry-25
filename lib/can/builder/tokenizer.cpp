#include "tokenizer.hpp"

#include <cstdlib>
#include <cstring>

#include "can_debug.hpp"

namespace can {

// Local buffer size for peekNextWord
static const std::size_t LOCAL_BUF_SIZE = 128;

// Prefix table (longest first)
struct PrefixEntry {
    const char* str;
    TokenType tt;
    std::size_t len;
};

static constexpr PrefixEntry prefixLUT[] = {
    {">>>>", TT_ENUM_PREFIX, 4},  {">>>", TT_SIGNAL_PREFIX, 3}, {"!!", TT_OPTION_PREFIX, 2},
    {">>", TT_MESSAGE_PREFIX, 2}, {">", TT_BOARD_PREFIX, 1},
};

static const std::size_t PREFIX_COUNT = sizeof(prefixLUT) / sizeof(prefixLUT[0]);

IdentifierPool::IdentifierPool() {}

IdentifierPoolHandle IdentifierPool::intern(const char* src) {
    _pool.emplace_back(src);
    return IdentifierPoolHandle{_pool.size() - 1};
}

const char* IdentifierPool::get(IdentifierPoolHandle h) const {
    if (h.index < _pool.size()) {
        return _pool[h.index].c_str();
    }
    CAN_DEBUG_PRINT_ERROR("Invalid IdentifierPoolHandle %zu", h.index);
    return "";
}

bool Tokenizer::start() {
    return _reader.start();
}

void Tokenizer::end() {
    _reader.end();
}

common::Option<Token> Tokenizer::next() {
    char buf[LOCAL_BUF_SIZE];
    std::size_t len = 0;

    // Skip blanks & comments
    while (true) {
        if (!_reader.peekNextWord(LOCAL_BUF_SIZE, buf, &len)) return common::Option<Token>::none();
        if (len > 0 && buf[0] == '#') {
            _reader.eatUntil('\n');
            continue;
        }
        break;
    }

    Token tk;
    // Try prefix match
    for (std::size_t i = 0; i < PREFIX_COUNT; ++i) {
        const auto& e = prefixLUT[i];
        if (len == e.len && std::memcmp(buf, e.str, e.len) == 0) {
            tk.type = e.tt;
            _reader.moveWord();
            return common::Option<Token>::some(tk);
        }
    }

    // Not a prefix: try numeric or identifier
    char* endptr = nullptr;
    // Hex integer? 0x...
    if (len > 2 && buf[0] == '0' && (buf[1] == 'x' || buf[1] == 'X')) {
        unsigned long long v = std::strtoull(buf, &endptr, 16);
        if (endptr == buf + len) {
            tk.type = TT_HEX_INT;
            tk.data.uintValue = v;
            _reader.moveWord();
            return common::Option<Token>::some(tk);
        }
    }
    // Decimal integer?
    long long iv = std::strtoll(buf, &endptr, 10);
    if (endptr == buf + len) {
        tk.type = TT_INT;
        tk.data.intValue = iv;
        _reader.moveWord();
        return common::Option<Token>::some(tk);
    }
    // Floating point?
    double dv = std::strtod(buf, &endptr);
    if (endptr == buf + len) {
        tk.type = TT_FLOAT;
        tk.data.floatValue = dv;
        _reader.moveWord();
        return common::Option<Token>::some(tk);
    }

    // Fallback: identifier
    tk.type = TT_IDENTIFIER;
    tk.data.idHandle = IdentifierPool::instance().intern(buf);

    _reader.moveWord();
    return common::Option<Token>::some(tk);
}

}  // namespace can
