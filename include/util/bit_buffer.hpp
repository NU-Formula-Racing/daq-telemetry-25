#ifndef __BIT_BUFFER_H__
#define __BIT_BUFFER_H__

#include <stddef.h>
#include <stdint.h>

#include "option.hpp"

struct BitBufferHandle {
    size_t size;
    size_t offset;
};

class BitBuffer {
   public:
    BitBuffer(size_t bitSize);
    ~BitBuffer();

    static BitBuffer empty() { return BitBuffer(0); }

    template <typename T>
    void write(BitBufferHandle handle, T value) {
        write(handle, &value, sizeof(T));
    }

    void write(BitBufferHandle handle, const void *data, size_t size);

    template <typename T>
    Option<T> read(BitBufferHandle handle) const {
        T value;
        if (read(handle, &value)) {
            return Option<T>(value);
        } else {
            return Option<T>::none();
        }
    }

    Option<void *> read(BitBufferHandle handle) const;
    bool read(BitBufferHandle handle, void *data) const;

    size_t bitSize() const { return _bitSize; }
    size_t byteSize() const { return (_bitSize + 7) / 8; }

   private:
    uint8_t *_buffer;
    size_t _bitSize;
};

#endif  // __BIT_BUFFER_H__