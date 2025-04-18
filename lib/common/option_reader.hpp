#ifndef __OPTION_READER_H__
#define __OPTION_READER_H__

#include <stdint.h>

#include <array>
#include <define.hpp>
#include <utility>

#define __ADC_VAL(voltage) static_cast<uint16_t>((v) * 4095.0f / 3.3f + 0.5f)
#define __MASK(bitNum) static_cast<uint8_t>((0x1 << bitNum))

namespace common {

class OptionReader {
   public:
    OptionReader(HWPin pin) : _pin(pin) {}

    bool opt(uint8_t optionNumber) {

    };

   private:
    HWPin _pin;
    static std::array<std::pair<uint16_t, uint8_t>, 16> __ADC_MAP;
};

std::array<std::pair<uint16_t, uint8_t>, 16> OptionReader::__ADC_MAP = {
    {__ADC_VAL(0), static_cast<uint8_t>(0)},                          // 0b0000
    {__ADC_VAL(1.8), __MASK(0)},                                      // 0b0001
    {__ADC_VAL(1.2), __MASK(1)},                                      // 0b0010
    {__ADC_VAL(2.5), __MASK(0) | __MASK(1)},                          // 0b0011
    {__ADC_VAL(0.9), __MASK(2)},                                      // 0b0100
    {__ADC_VAL(0), __MASK(0) | __MASK(2)},                            // 0b0101 -- test
    {__ADC_VAL(2.6), __MASK(1) | __MASK(2)},                          // 0b0110
    {__ADC_VAL(2.8), __MASK(0) | __MASK(1) | __MASK(2)},              // 0b0111
    {__ADC_VAL(0.7), __MASK(3)},                                      // 0b1000
    {__ADC_VAL(0), __MASK(0) | __MASK(3)},                            // 0b1001 -- test
    {__ADC_VAL(1.6), __MASK(1) | __MASK(3)},                          // 0b1010
    {__ADC_VAL(0), __MASK(0) | __MASK(1) | __MASK(3)},                // 0b1011 -- test
    {__ADC_VAL(0), __MASK(2) | __MASK(3)},                            // 0b1100 -- test
    {__ADC_VAL(0), __MASK(0) | __MASK(2) | __MASK(3)},                // 0b1101 -- test
    {__ADC_VAL(2.8), __MASK(1) | __MASK(2) | __MASK(3)},              // 0b1110
    {__ADC_VAL(3.0), __MASK(0) | __MASK(1) | __MASK(2) | __MASK(3)},  // 0b1111
};

}  // namespace common

#endif  // __OPTION_READER_H__