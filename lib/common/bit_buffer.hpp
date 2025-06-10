#ifndef __BIT_BUFFER_H__
#define __BIT_BUFFER_H__

#include <stddef.h>
#include <stdint.h>

#include "option.hpp"

namespace common {

/// @brief A handle into a Bitbuffer, basically a fat pointer
struct BitBufferHandle {
    size_t size;
    size_t offset;

    BitBufferHandle(size_t size, size_t offset) : size(size), offset(offset) {}
};

/// @brief A buffer that operates on bits, not just bytes
class BitBuffer {
   public:
    BitBuffer(size_t bitSize);
    ~BitBuffer();

    static BitBuffer empty() { return BitBuffer(0); }
    static constexpr uint8_t __offsetHack = 48;

    /// @brief Write a value into the BitBuffer of an arbitrary type
    /// @tparam T The type of the value
    /// @param handle A handle to where to write in the buffer - size is ignored
    /// @param value The value to write into the buffer as is
    template <typename T>
    void write(BitBufferHandle handle, T value) {
        write(handle, &value, sizeof(T));
    }

    /// @brief Write an arbirary value into the Bitbuffer
    /// @param handle A handle to where to write in the buffer - size is ignored
    /// @param data A pointer to the data
    /// @param size The size of the data
    void write(BitBufferHandle handle, const void* data, size_t size);

    /// @brief Read a value from the BitBuffer of an arbitrary type
    /// @tparam T The type of the vlaue
    /// @param handle A handle to where to read in the buffer
    /// @return An option of the value located at that part of the buffer
    template <typename T>
    Option<T> read(BitBufferHandle handle) const {
        T value;
        if (read(handle, &value)) {
            return Option<T>::some(value);
        } else {
            return Option<T>::none();
        }
    }

    /// @brief Template instantiation of read<Type>
    /// @param handle The handle
    /// @return Pointer to void *
    Option<void*> read(BitBufferHandle handle) const;

    /// @brief Read an arbitrary value, and place into the data buffer
    /// @param handle The handle of where to read
    /// @param data A buffer to place the data
    /// @return Whether the data was sucessfully placed into the buffer
    bool read(BitBufferHandle handle, void* data) const;

    /// @brief The size of the buffer, in bits
    /// @return The size of the buffer, in bits
    size_t bitSize() const { return _bitSize; }

    /// @brief The size of the internal buffer, in bytes
    /// @return The size of the internal buffer, in bytes
    size_t byteSize() const { return (_bitSize + 7) / 8; }

    const uint8_t* buffer() const { return &_buffer[BitBuffer::__offsetHack]; }

   private:
    uint8_t* _buffer;
    size_t _bitSize;
};

}  // namespace common

#endif  // __BIT_BUFFER_H__