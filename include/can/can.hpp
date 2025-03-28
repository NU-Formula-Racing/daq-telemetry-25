#ifndef __CAN_H__
#define __CAN_H__

#include <stdint.h>

#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

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
struct CANMessageRawPayload;

/**========================================================================
 *                           HAL INTERFACE
 *========================================================================**/

/// @brief The type of driver to use for the CAN bus.
enum DriverType {
    DT_POLLING,
    DT_INTERRUPT
};

class CANDriver {
   public:
    virtual DriverType getDriverType() = 0;
    virtual void install(CANBaudRate baudRate) = 0;
    virtual void uninstall() = 0;
    virtual void sendMessage(const CANMessage &message, const CANMessageRawPayload &payload) = 0;

    // message handling for interrupt-based drivers
    virtual void attachInterrupt(std::function<void(const CANMessage &)> callback) = 0;

    // message handling for polling-based drivers
    virtual bool hasReceivedMessage() = 0;
    virtual CANMessage receiveMessage() = 0;

    virtual void clearTransmitQueue() = 0;
    virtual void clearReceiveQueue() = 0;
};

class CANBus {
   public:
    CANBus(CANDriver &driver, CANBaudRate baudRate)
        : _driver(driver), _baudRate(baudRate), _nextBitOffset(0), _buffer(BitBuffer::empty()), _bufferMutex(),
        _signals(), _messages() {}

    ~CANBus();

    /**
     * @brief Adds a message to the CAN bus using the provided description.
     * Registers the onReceive callback (if provided) internally.
     * @param description The description of the message.
     * @return A reference to the newly added CANMessage.
     */
    CANMessage &addMessage(const CANMessageDescription &description);

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
    void registerCallback(uint32_t messageID, std::function<void(const CANMessage &)> callback);

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
    CANDriver &_driver;
    CANBaudRate _baudRate;
    size_t _nextBitOffset;
    std::vector<CANMessage> _messages;
    std::vector<CANSignal> _signals;
    BitBuffer _buffer;
    std::mutex _bufferMutex;

    // Maps CAN message IDs to their registered callback functions.
    std::unordered_map<uint32_t, std::function<void(const CANMessage &)>>
        _callbacks;

    BitBufferHandle allocateSignalHandle(uint8_t bitLength);
    BitBufferHandle allocateMessageHandle(uint8_t bitLength);
};

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
    std::vector<size_t> _signalIndicies;
    friend class CANBus;
};

struct CANMessageRawPayload {
    union {
        uint8_t data[8];
        uint64_t data64;
    };
};

}  // namespace can

#endif  // __CAN_H__
