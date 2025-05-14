#include "ack_q.hpp"
#include "message.hpp"

namespace common {



void AckQueue::addPending(const LoRaPacket& packet) {
    pendingAcks_.push(packet);
}

bool AckQueue::getNextPending(LoRaPacket& packet) {
    if (pendingAcks_.empty()) {
        return false;
    }
    packet = pendingAcks_.top();
    pendingAcks_.pop();
    return true;
}

void AckQueue::markAcked(const LoRaPacket& packet) {
    if (!pendingAcks_.empty() && pendingAcks_.top().sequenceNumber() == packet.sequenceNumber()) {
        pendingAcks_.pop();
    }
}

bool AckQueue::hasPending() const {
    return !pendingAcks_.empty();
}

void AckQueue::clear() {
    while (!pendingAcks_.empty()) {
        pendingAcks_.pop();
    }
}

}  // namespace common 