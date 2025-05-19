#include <can.hpp>
#include <cstdlib>
#include <cstring>
#include <vector>

using namespace can;
using namespace common;

CANBus::~CANBus() {
    _driver.uninstall();
}

CANMessage CANBus::addMessage(const CANMessageDescription& description) {
    // if we have already initialized the bus, we cannot add messages
    if (_isInitialized) {
        CAN_DEBUG_PRINT_ERROR("Cannot add messages after initialization.");
        return CANMessage{.bus = *this, .handle = BitBufferHandle::none()};
    }

    // increment our bit offset to the next byte boundary
    _nextBitOffset = (_nextBitOffset + 7) / 8 * 8;

    // allocate memory for the vector of CANSignals
    BitBufferHandle messageHandle(description.length * 8, _nextBitOffset);
    std::vector<CANSignal> signals(description.signals.size());

    // create a new message and add it to our list of messages
    CANMessage msg = (CANMessage){.bus = *this,
                                  .id = description.id,
                                  .length = description.length,
                                  .type = description.type,
                                  .bufferHandle = messageHandle,
                                  .signals = signals};

    _messages[description.id] = msg;

    size_t offset = _nextBitOffset;

    // now we need to add the signals to the message
    for (int i = 0; i < description.signals.size(); i++) {
        // create a new can signal and add it to our list of signals
        const CANSignalDescription& signalDescription = description.signals[i];
        BitBufferHandle signalHandle(signalDescription.length, offset + signalDescription.startBit);

        CANSignal signal = (CANSignal){.message = msg,
                                       .handle = signalHandle,
                                       .isSigned = signalDescription.isSigned,
                                       .endianness = signalDescription.endianness,
                                       .factor = signalDescription.factor,
                                       .offset = signalDescription.offset};

        signals[i] = signal;
    }

    if (description.onReceive) {
        registerCallback(description.id, description.onReceive);
    }

    return msg;
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
        totalBits += msg.second.length * 8;
    }

    CAN_DEBUG_PRINT("Allocating buffer of size %d for CAN!\n", totalBits);
    this->_buffer = BitBuffer(totalBits);
    this->_isInitialized = true;
}

void CANBus::sendMessage(CANMessage& message) const {
    RawCANMessage rawMessage = encodeMessage(message);
    this->_driver.sendMessage(rawMessage);
}

void CANBus::update() {
    RawCANMessage message;
    if (!_driver.receiveMessage(&message)) return;

    // do something with the message
}

void CANBus::registerCallback(uint32_t messageID, std::function<void(const CANMessage&)> callback) {
    _callbacks[messageID] = callback;
}

bool CANBus::validateMessages() {}

RawCANMessage CANBus::encodeMessage(const CANMessage& message) const {
    RawCANMessage res;
    res.data64 = 0;
}

RawCANMessage CANBus::decodeMessage(const CANMessage& message, const RawCANMessage& payload) const {

}