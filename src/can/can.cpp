#include "can/can.hpp"

#include <cstdlib>
#include <cstring>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

using namespace can;

CANBus::~CANBus() {
    _driver.uninstall();
}

// This helper function allocates space in the BitBuffer for a signal
// and returns a BitBufferhandle that points to the allocated bit region.
BitBufferHandle CANBus::allocateSignalHandle(uint8_t bitLength) {
    BitBufferHandle handle = {_nextBitOffset, _nextBitOffset + bitLength};
    _nextBitOffset += bitLength;
    return handle;
}

BitBufferHandle CANBus::allocateMessageHandle(uint8_t bitLength) {
    // we must align the message to the next byte boundary
    _nextBitOffset = (_nextBitOffset + 7) & ~7;
    BitBufferHandle handle = {_nextBitOffset, _nextBitOffset + bitLength};
    _nextBitOffset += bitLength;
    return handle;
}

CANMessage &CANBus::addMessage(const CANMessageDescription &description) {
    CANMessage &msg = _messages.back();
    // If a callback was provided, register it.
    if (description.onReceive) {
        registerCallback(description.id, description.onReceive);
    }

    return msg;
}

void CANBus::initialize() {
    this->_driver.install(this->_baudRate);
}

CANSignal CANBus::getSignal(size_t index) {
    return _signals[index];
}

void CANBus::sendMessage(CANMessage &message) {
    RawCANMessage rawMessage = encodeMessage(message);
    this->_driver.sendMessage(rawMessage);
}

// -----------------------------------------------------------------------------
// Callback Registration and Interrupt Handling
// -----------------------------------------------------------------------------

void CANBus::registerCallback(
    uint32_t messageID, std::function<void(const CANMessage &)> callback) {
    _callbacks[messageID] = callback;
}

bool CANBus::validateMessages() {
    // Ensure that all messages have the correct number of signals.
    for (auto &msg : _messages) {
        if (msg.length != msg._signalIndicies.size()) {
            return false;
        }
    }
    return true;
}
