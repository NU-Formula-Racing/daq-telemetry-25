#ifndef __CAN_H__
#define __CAN_H__

#include <stdint.h>

#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "driver/twai.h"
#include "util/bit_buffer.hpp"

namespace can {

/**========================================================================
 *                           ENUMS
 *========================================================================**/

enum Endianness { MSG_LITTLE_ENDIAN,
                  MSG_BIG_ENDIAN };
enum FrameType { STANDARD,
                 EXTENDED };
enum CANBaudRate {
    CAN_100KBPS,
    CAN_125KBPS,
    CAN_250KBPS,
    CAN_500KBPS,
    CAN_1MBPS
};

/**========================================================================
 *                           DESCRIPTION STRUCTURES
 *========================================================================**/

struct CANSignalDescription {
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

struct CANMessageDescription {
   public:
    uint32_t id;
    uint8_t length;
    FrameType type;
    std::vector<CANSignalDescription> signals;
    // Callback to be invoked when this message is received.
    std::function<void(const struct CANMessage &)> onReceive;
};

/**========================================================================
 *                           INTERNAL CLASSES
 *========================================================================**/

// Forward declarations
class CANBus;
class CANSignal;
class CANMessage;

class CANSignal {
   public:
    CANMessage &message;
    BitBufferHandle handle;
    bool is_signed;
    Endianness endianness;
    double factor;
    double offset;
    double minimum;

    // A simple constructor to ease creation.
    CANSignal(CANMessage &msg, BitBufferHandle h, bool sign, Endianness endian,
              double fac, double off, double minVal)
        : message(msg), handle(h), is_signed(sign), endianness(endian), factor(fac), offset(off), minimum(minVal) {}
};

class CANMessage {
   public:
    CANBus &bus;
    uint32_t id;
    uint8_t length;
    FrameType type;

    // Methods for setting and reading signals by index.
    template <typename T>
    void setSignalValue(size_t index, T value) {
        return bus.setSignalValue<T>(bus.getSignal(index), value);
    }

    template <typename T>
    Option<T> getSignalValue(size_t index) {
        return bus.getSignalValue<T>(bus.getSignal(index));
    }

    // Convenience: send this message over the bus.
    void sendMessage() { bus.sendMessage(*this); }

    // Constructor
    CANMessage(CANBus &busRef, uint32_t messageId, uint8_t len,
               FrameType frameType)
        : bus(busRef), id(messageId), length(len), type(frameType) {}

   private:
    // Holds indices into CANBus's internal _signals vector.
    std::vector<size_t> _signalIndicces;
    friend class CANBus;
};

/**========================================================================
 *                           CANBUS CLASS
 *========================================================================**/

class CANBus {
   public:
    /**
     * @brief Constructs a CANBus instance.
     * @param baudRate The baud rate to use.
     * @param txPin The ESP32 TX pin for the TWAI driver.
     * @param rxPin The ESP32 RX pin for the TWAI driver.
     */
    CANBus(CANBaudRate baudRate, gpio_num_t txPin, gpio_num_t rxPin);

    ~CANBus();

    /**
     * @brief Adds a message to the CAN bus using the provided description.
     * Registers the onReceive callback (if provided) internally.
     * @param description The description of the message.
     * @return A reference to the newly added CANMessage.
     */
    CANMessage &addMessage(const CANMessageDescription &description);

    /**
     * @brief Initializes the CAN bus hardware (TWAI driver), sets the pins, and
     * registers interrupts.
     */
    void initialize();

    /**
     * @brief Sends a CAN message.
     * @param message The CAN message to send.
     */
    void sendMessage(CANMessage &message);

    /**
     * @brief Registers a callback for a given message ID.
     * When a message with this ID is received, the callback will be invoked.
     * @param messageID The CAN message ID.
     * @param callback The function to call on receipt.
     */
    void registerCallback(uint32_t messageID,
                          std::function<void(const CANMessage &)> callback);

    /**
     * @brief Processes an interrupt from the TWAI driver.
     * This should be called from your ISR to handle incoming messages.
     */
    void handleInterrupt();

    template <typename T>
    Option<T> getSignalValue(CANSignal &signal) {
        std::lock_guard<std::mutex> lock(_bufferMutex);
        return _buffer.read<T>(signal.handle);
    }

    template <typename T>
    void setSignalValue(CANSignal &signal, T value) {
        std::lock_guard<std::mutex> lock(_bufferMutex);
        _buffer.write(signal.handle, value);
    }

    CANSignal &getSignal(size_t index) { return _signals[index]; }

    bool validateMessages();


   private:
    CANBaudRate _baudRate;
    gpio_num_t _txPin;
    gpio_num_t _rxPin;
    size_t _nextBitOffset;
    std::vector<CANMessage> _messages;
    std::vector<CANSignal> _signals;
    BitBuffer _buffer;
    std::mutex _bufferMutex;

    // Maps CAN message IDs to their registered callback functions.
    std::unordered_map<uint32_t, std::function<void(const CANMessage &)>>
        _callbacks;

    BitBufferHandle allocateSignalHandle(uint8_t bitLength);

    twai_general_config_t _genConfig;
    twai_timing_config_t _timingConfig;
    twai_filter_config_t _filterConfig;
};

};  // namespace can


/**========================================================================
 *                           CONVIENIENCE MACROS
 *========================================================================**/

#define CAN_MESSAGE_STANDARD(id, len, ...) \
    (can::CANMessageDescription){id, len, can::FrameType::STANDARD, .signals = { __VA_ARGS__ }}

#define CAN_MESSAGE_EXTENDED(id, len, ...) \
    (can::CANMessageDescription){id, len, can::FrameType::EXTENDED, .signals = { __VA_ARGS__ }}

#define CAN_SIGNAL(...) \
    (can::CANSignalDescription){__VA_ARGS__}

#define CAN_SIGNAL_START_BIT(start) \
    .start_bit = start

#define CAN_SIGNAL_START_AUTO \
    .start_bit = 0

#define CAN_SIGNAL_LENGTH(len) \
    .length = len

#define CAN_SIGNAL_LENGTH_AUTO(type) \
    .length = sizeof(type) * 8

#define CAN_SIGNAL_SIGNED() \
    .is_signed = true

#define CAN_SIGNAL_UNSIGNED() \
    .is_signed = false

#define CAN_SIGNAL_LITTLE_ENDIAN() \
    .endianness = can::Endianness::MSG_LITTLE_ENDIAN

#define CAN_SIGNAL_BIG_ENDIAN() \
    .endianness = can::Endianness::MSG_BIG_ENDIAN

#define CAN_SIGNAL_FACTOR(fac) \
    .factor = fac

#define CAN_SIGNAL_DEFAULT_FACTOR() \
    .factor = 1.0

#define CAN_SIGNAL_OFFSET(off) \
    .offset = off

#define CAN_SIGNAL_DEFAULT_OFFSET() \
    .offset = 0.0

#define CAN_SIGNAL_MINIMUM(min) \

#define CAN_SIGNAL_DEFAULT_MINIMUM() \
    .minimum = 0.0

#define CAN_SIGNAL_MAXIMUM(max) \
    .maximum = max

#define CAN_SIGNAL_DEFAULT_MAXIMUM() \
    .maximum = 255.0


#endif  // __CAN_H__
