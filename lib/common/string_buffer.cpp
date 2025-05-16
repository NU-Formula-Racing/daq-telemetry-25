#include <string_buffer.hpp>
#include <cstdlib>
#include <cstring>

using namespace common;

StringBuffer::StringBuffer(size_t initialCapacity) : _size(0), _capacity(initialCapacity) {
    _buffer = static_cast<char*>(malloc(_capacity));
    if (_buffer) {
        _buffer[0] = '\0';
    }
}

StringBuffer::~StringBuffer() {
    free(_buffer);
}

bool StringBuffer::_ensureCapacity(size_t additional) {
    if (_size + additional > _capacity) {
        size_t newCapacity = _capacity * 2;
        while (newCapacity < _size + additional) {
            newCapacity *= 2;
        }
        char* newBuffer = static_cast<char*>(realloc(_buffer, newCapacity));
        if (!newBuffer) {
            return false;
        }
        _buffer = newBuffer;
        _capacity = newCapacity;
    }
    return true;
}

Option<StringHandle> StringBuffer::append(const char* str) {
    if (!str) return Option<StringHandle>::none();

    size_t len = std::strlen(str) + 1;  // Include null terminator

    if (!_ensureCapacity(len)) {
        return Option<StringHandle>::none();
    }

    StringHandle handle{_size};
    std::memcpy(_buffer + _size, str, len);
    _size += len;

    return Option<StringHandle>::some(handle);
}

Option<const char*> StringBuffer::get(StringHandle handle) const {
    if (handle.offset >= _size) return Option<const char*>::none();
    return Option<const char*>::some(_buffer + handle.offset);
}
