#include <message.hpp>

namespace common {

LoRaPacket::LoRaPacket()
    : _isDirty(false) {
    _rawPacket.resize(HEADER_SIZE);
    _rawPacket[_rawPacket.size() - 1] = END_BYTE;
}

LoRaPacket::LoRaPacket(uint8_t msg_type, uint8_t options, uint16_t seq_num, const std::vector<uint8_t>& payload)
    : _isDirty(false) {
    auto result = checkPayloadSize(payload.size());
    if (result.isNone()) {
        return;
    }
    ensurePacketSize(payload.size());
    
    // Copy payload
    std::copy(payload.begin(), payload.end(), _rawPacket.begin());
    
    // Set header fields
    size_t offset = payload.size();
    _rawPacket[offset] = options;
    _rawPacket[offset + 1] = msg_type;
    _rawPacket[offset + 2] = static_cast<uint8_t>(seq_num >> 8);
    _rawPacket[offset + 3] = static_cast<uint8_t>(seq_num & 0xFF);
    
    updateCRC();
    _rawPacket[_rawPacket.size() - 1] = END_BYTE;
}

bool LoRaPacket::validatePacket(const std::vector<uint8_t>& raw) {
    if (raw.size() < HEADER_SIZE) return false;
    if (raw[raw.size() - 1] != END_BYTE) return false;
    
    uint16_t crc = computeCRC(std::vector<uint8_t>(raw.begin(), raw.end() - 3));
    uint16_t received_crc = (static_cast<uint16_t>(raw[raw.size() - 3]) << 8) | raw[raw.size() - 2];
    return crc == received_crc;
}

Option<void *> LoRaPacket::deserialize(const std::vector<uint8_t>& raw) {
    if (!validatePacket(raw)) {
        return Option<void *>::none();
    }
    _rawPacket = raw;
    _isDirty = false;
    return Option<void *>::some(nullptr);
}

const std::vector<uint8_t>& LoRaPacket::serialize() {
    if (_isDirty) {
        updateCRC();
        _rawPacket[_rawPacket.size() - 1] = END_BYTE;
        _isDirty = false;
    }
    return _rawPacket;
}

uint8_t LoRaPacket::messageType() const noexcept {
    return _rawPacket[payloadSize() + 1];
}

bool LoRaPacket::needsAck() const noexcept {
    OptionsByte opt{.raw = _rawPacket[payloadSize()]};
    return opt.needsAck == 1;
}

bool LoRaPacket::isAck() const noexcept {
    OptionsByte opt{.raw = _rawPacket[payloadSize()]};
    return opt.isAck == 1;
}

uint16_t LoRaPacket::sequenceNumber() const noexcept {
    size_t offset = payloadSize() + 2;
    return (static_cast<uint16_t>(_rawPacket[offset]) << 8) | _rawPacket[offset + 1];
}

std::vector<uint8_t> LoRaPacket::payload() const noexcept {
    return std::vector<uint8_t>(_rawPacket.begin(), _rawPacket.begin() + payloadSize());
}

size_t LoRaPacket::payloadSize() const noexcept {
    return _rawPacket.size() - HEADER_SIZE;
}

Option<void *> LoRaPacket::setPayload(const std::vector<uint8_t>& payload) {
    auto result = checkPayloadSize(payload.size());
    if (result.isNone()) {
        return result;
    }
    ensurePacketSize(payload.size());
    std::copy(payload.begin(), payload.end(), _rawPacket.begin());
    _isDirty = true;
    return Option<void *>::some(nullptr);
}

void LoRaPacket::setOptions(uint8_t options) {
    _rawPacket[payloadSize()] = options;
    _isDirty = true;
}

void LoRaPacket::setMessageType(uint8_t msg_type) {
    _rawPacket[payloadSize() + 1] = msg_type;
    _isDirty = true;
}

void LoRaPacket::setSequenceNumber(uint16_t seq_num) {
    size_t offset = payloadSize() + 2;
    _rawPacket[offset] = static_cast<uint8_t>(seq_num >> 8);
    _rawPacket[offset + 1] = static_cast<uint8_t>(seq_num & 0xFF);
    _isDirty = true;
}

uint16_t LoRaPacket::computeCRC(const std::vector<uint8_t>& data) {
    uint16_t crc = 0xFFFF;
    for (uint8_t byte : data) {
        crc ^= byte;
        for (int i = 0; i < 8; i++) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

Option<void *> LoRaPacket::checkPayloadSize(size_t size) {
    if (size > MAX_PAYLOAD) {
        return Option<void *>::none();
    }
    return Option<void *>::some(nullptr);
}

void LoRaPacket::ensurePacketSize(size_t payloadSize) {
    _rawPacket.resize(payloadSize + HEADER_SIZE);
}

void LoRaPacket::updateCRC() {
    uint16_t crc = computeCRC(std::vector<uint8_t>(_rawPacket.begin(), _rawPacket.end() - 3));
    _rawPacket[_rawPacket.size() - 3] = static_cast<uint8_t>(crc >> 8);
    _rawPacket[_rawPacket.size() - 2] = static_cast<uint8_t>(crc & 0xFF);
}

}  // namespace common
