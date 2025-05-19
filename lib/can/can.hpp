#ifndef __CAN_H__
#define __CAN_H__

#include <stdint.h>

#include <bit_buffer.hpp>
#include <functional>
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
    CANMessage addMessage(const CANMessageDescription& description);

    void initialize();

    void update();

    /// @brief Sends a CAN message.
    /// @param message The CAN message to send.
    void sendMessage(CANMessage& message) const;

    /// @brief Registers a callback for a given message ID.
    /// When a message with this ID is received, the callback will be invoked.
    /// @param messageID The CAN message ID.
    /// @param callback The function to call on receipt.
    void registerCallback(uint32_t messageID, std::function<void(const CANMessage&)> callback);

    /// @brief Validates the messages within the CANBus, ensuring that they meet the requirements of
    /// the DBC
    /// @return Whether the messages are valid or not
    bool validateMessages();

   private:
    // HAL
    CANDriver& _driver;
    CANBaudRate _baudRate;

    // CAN DBC
    std::unordered_map<uint32_t, CANMessage&> _messages;

    // Buffer management
    BitBuffer _buffer;
    std::mutex _bufferMutex;
    size_t _nextBitOffset;

    bool _isInitialized = false;

    // Maps CAN message IDs to their registered callback functions.
    std::unordered_map<uint32_t, std::function<void(const CANMessage&)>> _callbacks;

    RawCANMessage encodeMessage(const CANMessage& message) const;
    RawCANMessage decodeMessage(const CANMessage& message, const RawCANMessage& payload) const;
};

class CANSignal {
   public:
    const CANMessage& message;
    const BitBufferHandle handle;
    const bool isSigned;
    const Endianness endianness;
    const double factor;
    const double offset;

    CANSignal(const CANSignal& other) = default;
    CANSignal& operator=(CANSignal&& other) = default;
    CANSignal& operator=(const CANSignal& other) = default;

    template <typename T>
    void setValue(T value) const;
};

class CANMessage {
   public:
    const CANBus& bus;
    const uint32_t id;
    const uint8_t length;
    const FrameType type;
    const BitBufferHandle bufferHandle;
    const std::vector<CANSignal> signals;

    CANMessage(const CANMessage& other) = default;
    CANMessage& operator=(CANMessage&& other) = default;
    CANMessage& operator=(const CANMessage& other) = default;

    void sendMessage() { bus.sendMessage(*this); }
};

}  // namespace can

#endif  // __CAN_H__
