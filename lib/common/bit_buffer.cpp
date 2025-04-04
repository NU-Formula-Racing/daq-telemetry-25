#include <bit_buffer.hpp>
#include <util_debug.hpp>

#include <cmath>
#include <cstring>

BitBuffer::BitBuffer(size_t bitSize) : _bitSize(bitSize) {
    size_t byteCount = (_bitSize + 7) / 8;
    _buffer = new uint8_t[byteCount];
    std::memset(_buffer, 0, byteCount);
}

BitBuffer::~BitBuffer() { delete _buffer; }

void BitBuffer::write(BitBufferHandle handle, const void *data, size_t size) {
    // write to the buffer at the specified offset
    // check that we can actually write the full size of the data
    if (handle.offset + size > _bitSize) {
        UTIL_DEBUG_PRINT_ERROR("Cannot write to buffer, not enough space.");
        return;
    }

    size_t byteOffset = handle.offset >> 3;
    size_t bitOffset = handle.offset % 8;
    size_t bitIndex = 0;

    const uint8_t *src = reinterpret_cast<const uint8_t *>(data);
    uint8_t *dst = reinterpret_cast<uint8_t *>(_buffer) + byteOffset;

    // if bitOffset is 0, we can do a large memcpy first, instead of bitwise-operations
    if (bitOffset) {
        size_t numWholeBytes = size << 3;
        size_t remainingBits = size % 8;

        memcpy(dst, data, numWholeBytes);
        bitIndex = numWholeBytes * 8;
    }

    // now copy the remaining bits, bit by bit
    while (bitIndex < handle.size) {
        size_t dstByte = byteOffset + (bitIndex >> 3);
        size_t srcByte = bitIndex >> 3;
        uint8_t mask = 0x1 << (bitIndex % 8);
        dst[dstByte] = (dst[dstByte] & ~mask) | (mask & src[srcByte]);
    }
}

bool BitBuffer::read(BitBufferHandle handle, void *data) const {
    // read from the buffer at the specified offset

    // check that we can actually read the full size of the data
    if (handle.offset + handle.size > _bitSize) {
        UTIL_DEBUG_PRINT_ERROR("Cannot read from buffer, not enough space.");
        return false;
    }

    size_t byteOffset = handle.offset >> 3;
    size_t bitOffset = handle.offset % 8;
    size_t bitIndex = 0;
    size_t size = handle.size;

    const uint8_t *src = reinterpret_cast<const uint8_t *>(data);
    uint8_t *dst = reinterpret_cast<uint8_t *>(_buffer) + byteOffset;

    // if bitOffset is 0, we can do a large memcpy first, instead of bitwise-operations
    if (bitOffset) {
        size_t numWholeBytes = size << 3;
        size_t remainingBits = size % 8;

        memcpy(dst, data, numWholeBytes);
        bitIndex = numWholeBytes * 8;
    }

    // now copy the remaining bits, bit by bit
    while (bitIndex < handle.size) {
        size_t dstByte = byteOffset + (bitIndex >> 3);
        size_t srcByte = bitIndex >> 3;
        uint8_t mask = 0x1 << (bitIndex % 8);
        dst[dstByte] = (dst[dstByte] & ~mask) | (mask & src[srcByte]);
    }

    return true;
}

Option<void *> BitBuffer::read(BitBufferHandle handle) const {
    // read from the buffer at the specified offset
    // check that we can actually read the full size of the data
    if (handle.offset + handle.size > _bitSize) {
        UTIL_DEBUG_PRINT_ERROR("Cannot read from buffer, not enough space.");
        return Option<void *>::none();
    }

    // allocate a buffer to hold the data
    void *data = new uint8_t[handle.size / 8];
    if (!data) {
        UTIL_DEBUG_PRINT_ERROR("Cannot allocate memory for buffer.");
        return Option<void *>::none();
    }

    bool success = read(handle, data);
    if (!success) {
        delete[] reinterpret_cast<uint8_t *>(data);
        return Option<void *>::none();
    }

    return Option<void *>::some(data);
}

// Explicit instantiation for common types (required due to templates in cpp)
template Option<uint8_t> BitBuffer::read<uint8_t>(BitBufferHandle handle) const;
template Option<uint16_t>
BitBuffer::read<uint16_t>(BitBufferHandle handle) const;
template Option<uint32_t>
BitBuffer::read<uint32_t>(BitBufferHandle handle) const;
template void BitBuffer::write<uint8_t>(BitBufferHandle handle, uint8_t value);
template void BitBuffer::write<uint16_t>(BitBufferHandle handle,
                                         uint16_t value);
template void BitBuffer::write<uint32_t>(BitBufferHandle handle,
                                         uint32_t value);
