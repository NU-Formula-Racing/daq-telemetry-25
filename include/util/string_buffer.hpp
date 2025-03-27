#ifndef __STRING_BUFFER_H__
#define __STRING_BUFFER_H__

#include <stddef.h>
#include <stdint.h>

#include "util/option.hpp"

struct StringHandle {
  size_t offset; // offset from the beginning of the buffer
};

class StringBuffer {
public:
  StringBuffer(size_t initialCapacity = 128);
  ~StringBuffer();

  Option<StringHandle> append(const char *str);
  Option<const char *> get(StringHandle handle) const;

  size_t size() const { return _size; }
  size_t capacity() const { return _capacity; }
  const char *raw() const { return _buffer; }

private:
  char *_buffer;
  size_t _size;
  size_t _capacity;

  bool _ensureCapacity(size_t additional);
};

#endif // __STRING_BUFFER_H__
