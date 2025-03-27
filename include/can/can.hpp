#ifndef __CAN_H__
#define __CAN_H__

#include <stdint.h>

#include <string>

namespace can {

enum Endianness {
    MSG_LITTLE_ENDIAN,
    MSG_BIG_ENDIAN
};

enum FrameType {
    STANDARD,
    EXTENDED
};

struct CANSignal {
   public:
    uint8_t start_bit;
    uint8_t length;
    bool is_signed;
    FrameType frame_type;
    Endianness endianness;
    double factor;
    double offset;
    double minimum;
    double maximum;
    std::string unit;
};

class CANMessage {

}

class CANBus {
    
}


};  // namespace can

#endif  // __CAN_H__