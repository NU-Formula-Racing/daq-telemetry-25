#include <telemetry/message_controller.hpp>
#include <telemetry/message.hpp>
#include <telemetry/ack_q.hpp>
#include <iostream>

namespace common {

MessageController::MessageController()
    : _nextSequenceNumber(0) {
    _ackQueue = std::make_unique<AckQueue>();
}

bool MessageController::sendMessage(uint8_t msgType, const std::vector<uint8_t>& payload, bool needsAck) {
    LoRaPacket packet;
    packet.setMessageType(msgType);
    packet.setPayload(payload);
    packet.setOptions(needsAck ? LoRaPacket::OPT_NEEDS_ACK : 0);
    packet.setSequenceNumber(_nextSequenceNumber++);
    return sendPacket(packet);
}

bool MessageController::sendPacket(const LoRaPacket& packet) {
    //adding to ack queue
    if (packet.needsAck()) {
        _ackQueue->addPending(packet);
    }
    return true;
}

bool MessageController::processReceivedPacket(const std::vector<uint8_t>& rawPacket) {
    if (!LoRaPacket::validatePacket(rawPacket)) {
        return false;
    }

    LoRaPacket packet;
    auto result = packet.deserialize(rawPacket);
    if (result.isNone()) {
        return false;
    }

    if (packet.isAck()) {
        handleAck(packet);
    } else {
        handleMessage(packet);
    }

    return true;
}

void MessageController::handleAck(const LoRaPacket& packet) {
    _ackQueue->markAcked(packet);
}

void MessageController::handleMessage(const LoRaPacket& packet) {
    uint8_t msgType = packet.messageType();
    std::vector<uint8_t> payload = packet.payload();

    std::cout << "Received message type: " << static_cast<int>(msgType) << std::endl;
    std::cout << "Payload size: " << payload.size() << std::endl;
    std::cout << "Sequence number: " << packet.sequenceNumber() << std::endl;
}

}  // namespace common

