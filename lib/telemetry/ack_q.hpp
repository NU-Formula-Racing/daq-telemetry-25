#ifndef ACK_QUEUE_HPP
#define ACK_QUEUE_HPP

#include <queue>
#include <vector>
#include <telemetry/message.hpp>

namespace common {

class AckQueue {
public:
    void addPending(const LoRaPacket& packet);
    bool getNextPending(LoRaPacket& packet);
    void markAcked(const LoRaPacket& packet);
    bool hasPending() const;
    void clear();

private:
    struct PacketPriority {
        bool operator()(const LoRaPacket& a, const LoRaPacket& b) {
            return a.sequenceNumber() > b.sequenceNumber();
        }
    };

    std::priority_queue<LoRaPacket, std::vector<LoRaPacket>, PacketPriority> pendingAcks_;
};

}  // namespace common

#endif
