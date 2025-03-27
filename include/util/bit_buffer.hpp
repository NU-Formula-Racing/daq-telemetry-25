#ifndef __BIT_BUFFER_H__
#define __BIT_BUFFER_H__

#include <stddef.h>
#include <stdint.h>

#include "option.hpp"

struct BitBufferhandle {
    size_t size;
    size_t capacity;
};

class BitBuffer {
   public:
    BitBuffer(size_t bitSize);
    ~BitBuffer();

    template <typename T>
    void write(BitBufferhandle handle, T value);
    void write(BitBufferhandle handle, const void *data, size_t size);

    template <typename T>
    Option<T> read(BitBufferhandle handle) const;
    Option<void *> read(BitBufferhandle handle) const;
    bool read(BitBufferhandle handle, void *data) const;

    size_t bitSize() const { return _bitSize; }
    size_t byteSize() const { return (_bitSize + 7) / 8; }

   private:
    void *_buffer;
    size_t _bitSize;
};

#endif  // __BIT_BUFFER_H__