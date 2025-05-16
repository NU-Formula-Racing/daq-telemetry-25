#ifndef __LORA_DRIVER_H__
#define __LORA_DRIVER_H__

#include <RH_RF95.h>

#include <define.hpp>

namespace common {

enum LoRaDriverState { LDS_IDLE, LDS_LISTENING, LDS_TRANSMITTING };

enum LoRaFrequency {

};

enum LoRaPower {

};

enum LoRaSpreadingFactor {};

enum LoRaBandwidth {

};

struct LoRaDriverOptions {
    // pins
    HWPin csPin;
    HWPin interruptPin;
    HWPin resetPin;

    // frequency and power
    LoRaFrequency frequency;
    LoRaPower power;

    // LoRa specific options
    LoRaSpreadingFactor spreadingFactor;
    LoRaBandwidth bandwidth;

    std::function <
};

class LoRaDriver {
   public:
    LoRaDriver(LoRaDriverOptions options) : _config(options) {}

    void initialize();

    bool send(const uint8_t* data, uint8_t len);
    bool recieve(uint8_t* data, uint8_t len);

   private:
    LoRaDriverOptions _config;
    RH_RF95 _hal;
};

}  // namespace common

#endif  // __LORA_DRIVER_H__