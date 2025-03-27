#include "util/bit_buffer.hpp"

#include <cstring> // for memcpy
#include <new>     // for placement new if needed

BitBuffer::BitBuffer(size_t bitSize) : _bitSize(bitSize) {
  size_t byteCount = (_bitSize + 7) / 8;
  _buffer = new uint8_t[byteCount];
  std::memset(_buffer, 0, byteCount);
}

BitBuffer::~BitBuffer() { delete[] reinterpret_cast<uint8_t *>(_buffer); }

void BitBuffer::write(BitBufferhandle handle, const void *data, size_t size) {
  if (handle.size + size * 8 > handle.capacity || size == 0)
    return;

  size_t byteOffset = handle.size / 8;
  size_t bitOffset = handle.size % 8;

  uint8_t *dst = reinterpret_cast<uint8_t *>(_buffer) + byteOffset;
  const uint8_t *src = reinterpret_cast<const uint8_t *>(data);

  for (size_t i = 0; i < size; ++i) {
    uint8_t byte = src[i];
    if (bitOffset == 0) {
      dst[i] = byte;
    } else {
      dst[i] &= ~(0xFF << bitOffset);
      dst[i] |= byte << bitOffset;
      if (i + 1 < size) {
        dst[i + 1] &= (0xFF << bitOffset);
        dst[i + 1] |= byte >> (8 - bitOffset);
      }
    }
  }
}

bool BitBuffer::read(BitBufferhandle handle, void *data) const {
  if (handle.size + 8 > handle.capacity)
    return false;

  size_t byteOffset = handle.size / 8;
  size_t bitOffset = handle.size % 8;

  const uint8_t *src = reinterpret_cast<const uint8_t *>(_buffer) + byteOffset;
  uint8_t *dst = reinterpret_cast<uint8_t *>(data);

  if (bitOffset == 0) {
    *dst = *src;
  } else {
    *dst = (*src >> bitOffset) | (*(src + 1) << (8 - bitOffset));
  }

  return true;
}

Option<void *> BitBuffer::read(BitBufferhandle handle) const {
  if (handle.size >= handle.capacity)
    return Option<void *>::none();

  size_t byteOffset = handle.size / 8;
  if (byteOffset >= byteSize())
    return Option<void *>::none();

  return Option<void *>::some(reinterpret_cast<void *>(
      reinterpret_cast<uint8_t *>(_buffer) + byteOffset));
}

template <typename T> void BitBuffer::write(BitBufferhandle handle, T value) {
  write(handle, &value, sizeof(T));
}

template <typename T> Option<T> BitBuffer::read(BitBufferhandle handle) const {
  T value;
  if (!read(handle, &value))
    return Option<T>::none();
  return Option<T>::some(value);
}

// Explicit instantiation for common types (required due to templates in cpp)
template Option<uint8_t> BitBuffer::read<uint8_t>(BitBufferhandle handle) const;
template Option<uint16_t>
BitBuffer::read<uint16_t>(BitBufferhandle handle) const;
template Option<uint32_t>
BitBuffer::read<uint32_t>(BitBufferhandle handle) const;
template void BitBuffer::write<uint8_t>(BitBufferhandle handle, uint8_t value);
template void BitBuffer::write<uint16_t>(BitBufferhandle handle,
                                         uint16_t value);
template void BitBuffer::write<uint32_t>(BitBufferhandle handle,
                                         uint32_t value);
