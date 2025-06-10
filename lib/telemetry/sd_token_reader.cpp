#ifndef __PLATFORM_NATIVE

#include "sd_token_reader.hpp"

#include <SD.h>

#include <cctype>  // for std::isspace
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <sd_manager.hpp>
#include <string>

namespace remote {

SDTokenReader::SDTokenReader(FileGuard& gaurd) : _guard(gaurd) {}

bool SDTokenReader::start() {
    return true;
}

bool SDTokenReader::peekNextWord(std::size_t maxLength, char* charBuf, std::size_t* length) {
    auto opt = _guard.file();
    if (opt.isNone()) {
        return false;
    }

    auto file = opt.value();

    // Remember where we were
    auto origPos = file.position();
    // Skip leading whitespace
    while (file.available()) {
        int c = file.peek();
        if (c < 0 || !std::isspace(c)) break;
        file.read();
    }

    // Read up to maxLength non-space chars
    std::size_t len = 0;
    while (len < maxLength && file.available()) {
        int c = file.peek();
        if (c < 0 || std::isspace(c)) break;
        charBuf[len++] = static_cast<char>(file.read());
    }

    // Restore file position
    file.seek(origPos);
    // set the null termnator
    charBuf[len] = '\0';

    if (len == 0) return false;
    *length = len;

    return true;
}

bool SDTokenReader::moveWord(std::size_t stepSize) {
    auto opt = _guard.file();
    if (opt.isNone()) {
        return false;
    }

    auto file = opt.value();

    for (std::size_t step = 0; step < stepSize; ++step) {
        // Skip whitespace
        while (file.available()) {
            int c = file.peek();
            if (c < 0 || !std::isspace(c)) break;
            file.read();
        }
        // Skip the word itself
        if (!file.available()) return false;
        while (file.available()) {
            int c = file.peek();
            if (c < 0 || std::isspace(c)) break;
            file.read();
        }
    }
    return true;
}

bool SDTokenReader::eatUntil(const char character) {
    auto opt = _guard.file();
    if (opt.isNone()) {
        return false;
    }

    auto file = opt.value();

    while (file.available()) {
        char c = static_cast<char>(file.read());
        if (c == character) {
            return true;
        }
    }

    return false;
}

void SDTokenReader::end() {}

}  // namespace remote

#endif