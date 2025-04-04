#include <can.hpp>

#include <cstdlib>
#include <cstring>
#include <vector>


using namespace can;

CANBus::~CANBus() {
    _driver.uninstall();
}

CANMessage &CANBus::addMessage(const CANMessageDescription &description) {
    // if we have already initialized the bus, we cannot add messages
    if (_isInitialized) {
        CAN_DEBUG_PRINT_ERROR("Cannot add messages after initialization.");
        return _messages.back();
    }

    // calculate the total number of bits we need to store in our bit buffer
    // this is where all of the data for the messages will be stored
    // a few invariants tho:
    // 1. the bit buffer must be large enough to hold all of the messages
    // 2. signals within a message will be contiguous in the bit buffer, following the specifications
    //    of the descriptions provided
    // 3. the start of a CAN message will be aligned to a byte boundary, so we need to make sure that the
    //    first signal in a message is aligned to a byte boundary
    // 4. the bit buffer must be large enough to hold all of the signals in all of the messages

    // increment our bit offset to the next byte boundary
    _nextBitOffset = (_nextBitOffset + 7) / 8 * 8;

    // create a new message and add it to our list of messages
    CANMessage msg(*this, description.id, _nextBitOffset, description.length, description.type);
    _messages.push_back(msg);

    size_t offset = _nextBitOffset;

    // now we need to add the signals to the message
    for (auto &signal : description.signals) {
        // create a new can signal and add it to our list of signals
        BitBufferHandle handle = {
            .size = signal.length,
            .offset = offset + signal.startBit,
        };

        CANSignal canSignal(msg, handle, signal.isSigned, signal.endianness,
                            signal.factor, signal.offset, signal.minimum);

        size_t currentIndex = _messages.size() - 1;
        _signals.push_back(canSignal);

        // push back the signal index to the message
        msg.signalIndicies.push_back(currentIndex);
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

    // calculate the total number of bits we need to store in our bit buffer
    // this is where all of the data for the messages will be stored
    // a few invariants tho:
    // 1. the bit buffer must be large enough to hold all of the messages
    // 2. signals within a message will be contiguous in the bit buffer, following the specifications
    //    of the descriptions provided
    // 3. the start of a CAN message will be aligned to a byte boundary, so we need to make sure that the
    //    first signal in a message is aligned to a byte boundary
    // 4. the bit buffer must be large enough to hold all of the signals in all of the messages

    size_t totalBits = 0;
    for (auto &msg : _messages) {
        totalBits += msg.length * 8;
    }

    this->_isInitialized = true;
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
        if (msg.length != msg.signalIndicies.size()) {
            return false;
        }
    }
    return true;
}

RawCANMessage CANBus::encodeMessage(const CANMessage &message) const {
    RawCANMessage res;
    res.data64 = 0;

    uint8_t bitIndex = 0;
    for (auto index : message.signalIndicies) {
        CANSignal signal = _signals[index];
        uint64_t sigBuf = _buffer.read(signal.handle);
        sigBuf = sigBuf * signal.factor + signal.offset;
        // copy the sigBuf into the data64 at the bit offset
    }

    return res;
}

RawCANMessage CANBus::decodeMessage(const CANMessage &message, const RawCANMessage &payload) const {

}