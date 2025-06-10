#include <can.hpp>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <memory>
#include <mutex>
#include <vector>

using namespace can;
using namespace common;

CANBus::~CANBus() {
    _driver.uninstall();
}

CANMessage& CANBus::addMessage(const CANMessageDescription& desc) {
    if (_isInitialized) {
        CAN_DEBUG_PRINT_ERROR("Cannot add messages after initialization.");
    }

    BitBufferHandle messageHandle(desc.length * 8, _nextBitOffset);
    _nextBitOffset += 64;

    // construct on the heap using the ctor without trailing-_ names
    std::unique_ptr<CANMessage> msgPtr =
        std::unique_ptr<CANMessage>(new CANMessage(*this,         // bus
                                                   desc.id,       // id
                                                   desc.length,   // length
                                                   desc.type,     // type
                                                   messageHandle  // bufferHandle
                                                   ));
    auto& msg = *msgPtr;  // stable reference

    msg.signals.reserve(desc.signals.size());
    for (auto const& sd : desc.signals) {
        BitBufferHandle h(sd.length, messageHandle.offset + sd.startBit);
        msg.signals.emplace_back(msg,            // message
                                 h,              // handle
                                 sd.isSigned,    // isSigned
                                 sd.endianness,  // endianness
                                 sd.factor,      // factor
                                 sd.offset       // offset
        );
    }

    if (desc.onReceive) {
        registerCallback(desc.id, desc.onReceive);
    }

    auto it = _messages.emplace(desc.id, std::move(msgPtr)).first;
    return *it->second;
}

void CANBus::initialize() {
    CAN_DEBUG_PRINTLN("Initializing CANBus");
    if (_isInitialized) {
        CAN_DEBUG_PRINT_ERROR("CANBus is already initialized.");
        return;
    }

    this->_driver.install(this->_baudRate);

    // calculate the amount of bits that we need for the CANSignal
    size_t totalBits = _nextBitOffset;

    CAN_DEBUG_PRINT("Allocating buffer of size %d for CAN!\n", totalBits);
    this->_buffer = BitBuffer(totalBits);
    this->_isInitialized = true;
}

void CANBus::sendMessage(const CANMessage& message) {
    RawCANMessage rawMessage = getRawMessage(message);
    this->_driver.sendMessage(rawMessage);
}

void CANBus::update() {
    RawCANMessage rawMessage;
    uint8_t numRx;

    while (_driver.receiveMessage(&rawMessage)) {
        numRx++;
        if (_messages.find(rawMessage.id) == _messages.end())
            continue;  // we don't care about this message

        std::unique_ptr<CANMessage>& message = _messages[rawMessage.id];

        // CAN_DEBUG_PRINTLN("Storing message at offset %d", message->bufferHandle.offset);

        // write it into the buffer
        {
            std::lock_guard<std::mutex> lk(_bufferMutex);
            BitBufferHandle handle(64, message->bufferHandle.offset);
            _buffer.write(handle, rawMessage.data64);
        }


        if (numRx > 32) {
            CAN_DEBUG_PRINT_ERRORLN("Breaking early from update!");
        }
    }
}

void CANBus::registerCallback(uint32_t messageID, std::function<void(const CANMessage&)> callback) {
    _callbacks[messageID] = callback;
}

bool CANBus::validateMessages() {
    return true;
}

void CANBus::printBus(std::ostream& stream) const {
    stream << "*** BUS BEGIN ***" << std::endl;

    // Iterate all registered messages
    for (const auto& entry : _messages) {
        const CANMessage& msg = *entry.second;

        // Print message header
        stream << "Message ID=0x" << std::hex << std::setw(3) << std::setfill('0') << msg.id
               << std::dec << "  Length=" << int(msg.length) << " bytes" << std::endl;

        // Iterate signals within this message
        for (size_t idx = 0; idx < msg.signals.size(); ++idx) {
            const CANSignal& sig = msg.signals[idx];
            // Access BitBufferHandle fields
            size_t offset = sig.handle.offset;
            size_t size = sig.handle.size;

            stream << "  Signal[" << idx << "]: "
                   << "offset=" << offset << ", width=" << size << " bits"
                   << ", signed=" << std::boolalpha << sig.isSigned << std::noboolalpha
                   << ", endianness=" << (sig.endianness == MSG_BIG_ENDIAN ? "big" : "little")
                   << ", factor=" << sig.factor << ", offset=" << sig.offset << std::endl;
        }
    }

    stream << "*** BUS END ***" << std::endl;
}

const std::unordered_map<uint32_t, std::unique_ptr<CANMessage>>& CANBus::getMessages() const {
    return _messages;
}

RawCANMessage CANBus::getRawMessage(const CANMessage& message) {
    RawCANMessage raw{};
    raw.id = message.id;
    raw.length = message.length;
    raw.data64 = 0;

    // snapshot exactly `length` bytes out of our bit-buffer
    {
        std::lock_guard<std::mutex> lk(_bufferMutex);
        uint8_t tmp[8] = {0};
        _buffer.read(message.bufferHandle, tmp);
        std::memcpy(raw.data, tmp, raw.length);
    }

    return raw;
}

bool CANBus::writeRawMessage(const RawCANMessage raw) {
    // find the message by ID
    auto it = _messages.find(raw.id);
    if (it == _messages.end()) return false;
    auto& msg = *it->second;

    // write into our bit-buffer
    {
        std::lock_guard<std::mutex> lk(_bufferMutex);
        _buffer.write(msg.bufferHandle, raw.data, raw.length);
    }

    // invoke user callback if registered
    auto cb = _callbacks.find(raw.id);
    if (cb != _callbacks.end()) {
        cb->second(msg);
    }

    return true;
}
