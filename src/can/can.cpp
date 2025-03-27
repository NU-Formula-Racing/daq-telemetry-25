#include "can/can.hpp"

#include <cstdlib>
#include <cstring>

#include "driver/gpio.h"
#include "driver/twai.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

using namespace can;

CANBus::CANBus(CANBaudRate baudRate, gpio_num_t txPin, gpio_num_t rxPin)
    : _baudRate(baudRate), _txPin(txPin), _rxPin(rxPin), _buffer(BitBuffer::empty()), _nextBitOffset(0) {}

CANBus::~CANBus() {
    // Stop the TWAI driver and uninstall it.
    twai_stop();
    twai_driver_uninstall();
}

// This helper function allocates space in the BitBuffer for a signal
// and returns a BitBufferhandle that points to the allocated bit region.
BitBufferHandle CANBus::allocateSignalHandle(uint8_t bitLength) {
    BitBufferHandle handle = {_nextBitOffset, _nextBitOffset + bitLength};
    _nextBitOffset += bitLength;
    _buffer = BitBuffer(_nextBitOffset);
    return handle;
}

CANMessage &CANBus::addMessage(const CANMessageDescription &description) {
    // Create a new message with given parameters.
    _messages.emplace_back(*this, description.id, description.length,
                           description.type);
    CANMessage &msg = _messages.back();

    // For each signal description, allocate a signal and add its index to the
    // message.
    for (const auto &sigDesc : description.signals) {
        BitBufferHandle handle = allocateSignalHandle(sigDesc.length);
        // Create a new CANSignal
        CANSignal signal =
            CANSignal(msg, handle, sigDesc.is_signed, sigDesc.endianness,
                      sigDesc.factor, sigDesc.offset, sigDesc.minimum);
        _signals.push_back(signal);
        msg._signalIndicces.push_back(_signals.size() - 1);
    }

    // If a callback was provided, register it.
    if (description.onReceive) {
        registerCallback(description.id, description.onReceive);
    }
    return msg;
}

void CANBus::initialize() {
    // Configure the TWAI driver.
    _genConfig = TWAI_GENERAL_CONFIG_DEFAULT(_txPin, _rxPin, TWAI_MODE_NORMAL);

    switch (_baudRate) {
        case CAN_100KBPS:
            _timingConfig = TWAI_TIMING_CONFIG_100KBITS();
            break;
        case CAN_125KBPS:
            _timingConfig = TWAI_TIMING_CONFIG_125KBITS();
            break;
        case CAN_250KBPS:
            _timingConfig = TWAI_TIMING_CONFIG_250KBITS();
            break;
        case CAN_500KBPS:
            _timingConfig = TWAI_TIMING_CONFIG_500KBITS();
            break;
        case CAN_1MBPS:
            _timingConfig = TWAI_TIMING_CONFIG_1MBITS();
            break;
        default:
            _timingConfig = TWAI_TIMING_CONFIG_100KBITS();
            break;
    }

    _filterConfig = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    // Install the TWAI driver.
    twai_driver_install(&_genConfig, &_timingConfig, &_filterConfig);

    // Start the TWAI driver.
    twai_start();
}

void CANBus::sendMessage(CANMessage &message) {
    // Prepare a TWAI message.
    twai_message_t twaiMsg;
    std::memset(&twaiMsg, 0, sizeof(twaiMsg));
    twaiMsg.identifier = message.id;
    twaiMsg.extd = (message.type == EXTENDED);
    twaiMsg.data_length_code = message.length;

    // For simplicity, assume that each signal is stored byte-aligned.
    // We read each signal from the BitBuffer and pack sequentially.
    uint8_t data[8] = {0};  // maximum data length
    for (size_t i = 0; i < message._signalIndicces.size() && i < message.length;
         ++i) {
        CANSignal signal = _signals[message._signalIndicces[i]];
        Option<uint8_t> optVal = getSignal<uint8_t>(signal);
        if (optVal) {
            data[i] = optVal;
        }
    }
    std::memcpy(twaiMsg.data, data, message.length);

    // Transmit the message (using a timeout of 1000ms).
    twai_transmit(&twaiMsg, pdMS_TO_TICKS(1000));
}

// -----------------------------------------------------------------------------
// Callback Registration and Interrupt Handling
// -----------------------------------------------------------------------------

void CANBus::registerCallback(
    uint32_t messageID, std::function<void(const CANMessage &)> callback) {
    _callbacks[messageID] = callback;
}

void CANBus::handleInterrupt() {
    // Called from the ISR or a polling task.
    twai_message_t twaiMsg;
    // Try to receive a message (non-blocking).
    if (twai_receive(&twaiMsg, 0) == ESP_OK) {
        // Look for a message with a matching ID in our registered messages.
        for (auto &msg : _messages) {
            if (msg.id == twaiMsg.identifier) {
                // Update each signal for the message.
                // (For simplicity, we assume signals are stored in order and are
                // byte-aligned.)
                for (size_t i = 0;
                     i < msg._signalIndicces.size() && i < twaiMsg.data_length_code;
                     ++i) {
                    CANSignal signal = _signals[msg._signalIndicces[i]];
                    // Write the received byte into the BitBuffer using setSignal.
                    setSignalValue<uint8_t>(signal, twaiMsg.data[i]);
                }
                // If a callback is registered, invoke it.
                auto it = _callbacks.find(msg.id);
                if (it != _callbacks.end()) {
                    it->second(msg);
                }
                break;  // assume unique message IDs
            }
        }
    }
}
