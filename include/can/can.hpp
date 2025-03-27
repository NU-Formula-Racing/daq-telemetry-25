#ifndef __CAN_H__
#define __CAN_H__

#include <stdint.h>

#include <string>
#include <vector>

#include "util/bit_buffer.hpp"

namespace can {

enum Endianness { MSG_LITTLE_ENDIAN, MSG_BIG_ENDIAN };

enum FrameType { STANDARD, EXTENDED };

struct CANSignal {
public:
  uint8_t start_bit;
  uint8_t length;
  bool is_signed;
  Endianness endianness;
  double factor;
  double offset;
  double minimum;
  double maximum;
};

class CANMessageDescription {
public:
  uint32_t id;
  FrameType type;
  uint8_t length;
  std::vector<CANSignal> signals;
};

class CANBus {
public:
private:
  BitBuffer _buffer;
};

}; // namespace can

#endif // __CAN_H__