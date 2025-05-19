#ifndef __CAN_H__
#define __CAN_H__

#include <stdint.h>

#include <bit_buffer.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "can_debug.hpp"

using namespace common;

namespace can {

enum Endianness { MSG_LITTLE_ENDIAN, MSG_BIG_ENDIAN };
enum FrameType { STANDARD, EXTENDED };
enum CANBaudRate { CBR_100KBPS, CBR_125KBPS, CBR_250KBPS, CBR_500KBPS, CBR_1MBPS };

/// @brief A descripton of a CANSignal, used purely for interface purposes with the CAN Bus
struct CANSignalDescription {
   public:
    uint8_t startBit;
    uint8_t length;
    bool isSigned;
    Endianness endianness;
    double factor;
    double offset;
};

/// @brief A description of a CANMessaged, used purely for interface purposes with the CAN Bus
struct CANMessageDescription {
   public:
    uint32_t id;
    uint8_t length;
    FrameType type;
    std::vector<CANSignalDescription> signals;

    // Callback to be invoked when this message is received.
    std::function<void(const struct CANMessage&)> onReceive;
};

// Forward declarations
class CANBus;
class CANSignal;
class CANMessage;
struct RawCANMessage;

/// @brief The type of driver to use for the CAN bus.
enum DriverType { DT_POLLING, DT_INTERRUPT, DT_NONE };

/// @brief The raw representation of a CAN Message, used for interfacing with lower-level drivers
struct RawCANMessage {
    uint32_t id;
    uint8_t length;
    union {
        uint8_t data[8];
        uint64_t data64;
    };
};

/// @brief An abstract class over a CAN driver, used to support multiple platforms
class CANDriver {
   public:
    CANDriver() {}

    virtual DriverType getDriverType() { return DT_NONE; }

    virtual void install(CANBaudRate baudRate) { /* no-op */ }
    virtual void uninstall() { /* no-op */ }
    virtual void sendMessage(RawCANMessage& message) { /* no-op */ }

    // message handling for interrupt-based drivers
    virtual void attachInterrupt(std::function<void(const CANMessage&)> callback) {
        // Default implementation does nothing.
    }

    bool receiveMessage(RawCANMessage* res) { return false; }

    virtual void clearTransmitQueue() { /* no-op */ }
    virtual void clearReceiveQueue() { /* no-op */ }
};

/// @brief A management class for CAN messages, abstracting over the DBC (provided by adding
/// messages), the driver, and compactly storing CAN messages in memory
class CANBus {
   public:
    CANBus(CANDriver& driver, CANBaudRate baudRate)
        : _driver(driver), _baudRate(baudRate), _nextBitOffset(0), _buffer(BitBuffer::empty()) {}

    ~CANBus();

    // no copies or moves allowed (references can’t be rebound)
    CANBus(const CANBus&) = delete;
    CANBus& operator=(const CANBus&) = delete;
    CANBus(CANBus&&) = delete;
    CANBus& operator=(CANBus&&) = delete;

    /// @brief Adds a message to the CAN bus using the provided description.
    /// Registers the onReceive callback (if provided) internally.
    /// @param description The description of the message.
    /// @return The newly added CAN Message
    CANMessage& addMessage(const CANMessageDescription& description);

    void initialize();

    void update();

    /// @brief Sends a CAN message.
    /// @param message The CAN message to send.
    void sendMessage(const CANMessage& message);

    /// @brief Registers a callback for a given message ID.
    /// When a message with this ID is received, the callback will be invoked.
    /// @param messageID The CAN message ID.
    /// @param callback The function to call on receipt.
    void registerCallback(uint32_t messageID, std::function<void(const CANMessage&)> callback);

    /// @brief Validates the messages within the CANBus, ensuring that they meet the requirements of
    /// the DBC
    /// @return Whether the messages are valid or not
    bool validateMessages();

    /// @brief Set the value of a signal
    /// @tparam T The type of the value
    /// @param signal The signal to set the value of
    /// @param value The value to set
    template <typename T>
    void setSignalValue(const CANSignal& signal, T value);

    /// @brief Get the value of a signal
    /// @tparam T the type of the vlaue
    /// @param signal The signal to get the value of
    /// @return The value of the signal
    template <typename T>
    T getSignalValue(const CANSignal& signal);

    /// @brief Prints out all of the messages on the bus
    /// @param stream The stream to print it to
    void printBus(std::ostream& stream) const;

   private:
    // HAL
    CANDriver& _driver;
    CANBaudRate _baudRate;

    // CAN DBC
    std::unordered_map<uint32_t, std::unique_ptr<CANMessage>> _messages;

    // Buffer management
    // holds the encoded values that are sent over can
    BitBuffer _buffer;
    std::mutex _bufferMutex;
    size_t _nextBitOffset;

    bool _isInitialized = false;

    // Maps CAN message IDs to their registered callback functions.
    std::unordered_map<uint32_t, std::function<void(const CANMessage&)>> _callbacks;

    RawCANMessage getRawMessage(const CANMessage& message);
    bool writeRawMessage(const RawCANMessage raw);
};

class CANSignal {
   public:
    const CANMessage& message;  // non-owning back-ref
    const BitBufferHandle handle;
    const bool isSigned;
    const Endianness endianness;
    const double factor;
    const double offset;

    // ctor uses same names as members
    CANSignal(const CANMessage& message, BitBufferHandle handle, bool isSigned,
              Endianness endianness, double factor, double offset) noexcept
        : message(message),
          handle(handle),
          isSigned(isSigned),
          endianness(endianness),
          factor(factor),
          offset(offset) {}

    CANSignal() = delete;
    CANSignal& operator=(const CANSignal&) = delete;

    template <typename T>
    void setValue(T value);

    template <typename T>
    T getValue();
};

class CANMessage {
   public:
    CANBus& bus;  // parent bus
    const uint32_t id;
    const uint8_t length;
    const FrameType type;
    const BitBufferHandle bufferHandle;
    std::vector<CANSignal> signals;  // mutable so we can fill it once

    // ctor uses same names as members
    CANMessage(CANBus& bus, uint32_t id, uint8_t length, FrameType type,
               BitBufferHandle bufferHandle) noexcept
        : bus(bus), id(id), length(length), type(type), bufferHandle(bufferHandle), signals() {}

    CANMessage() = delete;
    CANMessage& operator=(const CANMessage&) = delete;

    void sendMessage() { bus.sendMessage(*this); }
};

template <typename T>
void CANBus::setSignalValue(const CANSignal& signal, T value) {
    // 1) scale down to raw integer
    using RawType = typename std::conditional<signal.isSigned, int64_t, uint64_t>::type;
    double normalized = (static_cast<double>(value) - signal.offset) / signal.factor;
    RawType raw = static_cast<RawType>(normalized);

    // 2) write into the big buffer
    std::lock_guard<std::mutex> lk(_bufferMutex);
    _buffer.write(signal.handle, raw);
}

template <typename T>
T CANBus::getSignalValue(const CANSignal& signal) {
    // 1) read the raw bits back out
    using RawType = typename std::conditional<signal.isSigned, int64_t, uint64_t>::type;
    RawType raw = 0;
    {
        std::lock_guard<std::mutex> lk(_bufferMutex);
        _buffer.read(signal.handle, &raw);
    }

    // 2) if signed, sign-extend
    if (signal.isSigned) {
        raw = static_cast<RawType>((static_cast<uint64_t>(raw) << (64 - signal.handle.size)) >>
                                   (64 - signal.handle.size));
    }

    // 3) apply factor + offset
    double v = static_cast<double>(raw) * signal.factor + signal.offset;
    return static_cast<T>(v);
}

template <typename T>
void CANSignal::setValue(T value) {
    message.bus.setSignalValue(*this, value);
}

template <typename T>
T CANSignal::getValue() {
    return message.bus.getSignalValue<T>(*this);
}

}  // namespace can

#endif  // __CAN_H__
