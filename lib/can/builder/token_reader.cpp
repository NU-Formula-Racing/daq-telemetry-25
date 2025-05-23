#include "token_reader.hpp"

#include <cctype>
#include <cstring>

using can::MockTokenReader;

MockTokenReader::MockTokenReader(const std::string& configContents)
    : _content(configContents), _pos(0) {}

bool MockTokenReader::start() {
    _pos = 0;
    return true;
}

bool MockTokenReader::peekNextWord(size_t maxLength, char* charBuf, size_t* length) {
    size_t pos = _pos;
    size_t n = _content.size();

    // skip leading whitespace
    while (pos < n && std::isspace(static_cast<unsigned char>(_content[pos]))) {
        ++pos;
    }
    if (pos >= n) {
        return false;
    }

    // mark word boundaries
    size_t start = pos;
    while (pos < n && !std::isspace(static_cast<unsigned char>(_content[pos]))) {
        ++pos;
    }
    size_t wordLen = pos - start;

    // copy up to maxLength-1 chars + null terminator
    size_t copyLen = (wordLen < maxLength - 1) ? wordLen : (maxLength - 1);
    std::memcpy(charBuf, _content.data() + start, copyLen);
    charBuf[copyLen] = '\0';

    if (length) {
        *length = wordLen;
    }
    return true;
}

bool MockTokenReader::moveWord(size_t stepSize) {
    size_t n = _content.size();
    for (size_t i = 0; i < stepSize; ++i) {
        // skip whitespace
        while (_pos < n && std::isspace(static_cast<unsigned char>(_content[_pos]))) {
            ++_pos;
        }
        // if no more words
        if (_pos >= n) {
            return false;
        }
        // skip the word
        while (_pos < n && !std::isspace(static_cast<unsigned char>(_content[_pos]))) {
            ++_pos;
        }
    }
    return true;
}

bool MockTokenReader::eatUntil(const char character) {
    size_t n = _content.size();

    while (_pos < n && _content[_pos] != character) {
        ++_pos;
    }
    // if no more words
    if (_pos >= n) {
        return false;
    }

    return true;
}

void MockTokenReader::end() {
    _pos = _content.size();
}
