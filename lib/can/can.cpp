#include <can.hpp>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>

using namespace can;
using namespace common;

CANBus::~CANBus() {
    _driver.uninstall();
}

CANMessage& CANBus::addMessage(const CANMessageDescription& desc) {
    if (_isInitialized) {
        CAN_DEBUG_PRINT_ERROR("Cannot add messages after initialization.");
        // handle error...
    }

    _nextBitOffset = ((_nextBitOffset + 7) / 8) * 8;
    BitBufferHandle messageHandle(desc.length * 8, _nextBitOffset);

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
    if (_isInitialized) {
        CAN_DEBUG_PRINT_ERROR("CANBus is already initialized.");
        return;
    }

    this->_driver.install(this->_baudRate);

    // calculate the amount of bits that we need for the CANSignal
    size_t totalBits = 0;
    for (auto& msg : _messages) {
        totalBits += msg.second->length * 8;
    }

    CAN_DEBUG_PRINT("Allocating buffer of size %d for CAN!\n", totalBits);
    this->_buffer = BitBuffer(totalBits);
    this->_isInitialized = true;
}

void CANBus::sendMessage(const CANMessage& message) const {
    RawCANMessage rawMessage = encodeMessage(message);
    this->_driver.sendMessage(rawMessage);
}

void CANBus::update() {
    RawCANMessage message;
    if (!_driver.receiveMessage(&message)) return;

    // figure out what message it is, decode it, then store it in the buffer
}

void CANBus::registerCallback(uint32_t messageID, std::function<void(const CANMessage&)> callback) {
    _callbacks[messageID] = callback;
}

bool CANBus::validateMessages() {
    return true;
}

RawCANMessage CANBus::encodeMessage(const CANMessage& message) const {
    RawCANMessage res;
    res.data64 = 0;
}

RawCANMessage CANBus::decodeMessage(const CANMessage& message, const RawCANMessage& payload) const {

}