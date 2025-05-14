#ifndef MESSAGE_CONTROLLER_HPP
#define MESSAGE_CONTROLLER_HPP

#include <vector>
#include <memory>
#include <telemetry/message.hpp>
#include <telemetry/ack_q.hpp>

namespace common {

class MessageController {
public:
    MessageController();
    bool sendMessage(uint8_t msgType, const std::vector<uint8_t>& payload, bool needsAck = false);
    bool processReceivedPacket(const std::vector<uint8_t>& rawPacket);
    bool sendPacket(const LoRaPacket& packet);
    void handleAck(const LoRaPacket& packet);
    void handleMessage(const LoRaPacket& packet);
    bool hasPendingAck() const { return _ackQueue->hasPending(); }

private:
    uint16_t _nextSequenceNumber;
    std::unique_ptr<AckQueue> _ackQueue;
};

}  // namespace common

#endif 
