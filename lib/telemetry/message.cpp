#include <message.hpp>

LoRaPacket::LoRaPacket(uint8_t msg_type,
                       uint8_t options,
                       uint16_t seq_num,
                       std::vector<uint8_t> payload)
    : _msgType(msg_type),
      _options{.raw = options},
      _seqNum(seq_num),
      _payloadSize(payload.size()),
      _payload(std::move(payload)) {}

LoRaPacket::LoRaPacket()
    : _msgType(0),
      _options{.raw = 0},
      _seqNum(0),
      _payloadSize(0),
      _payload() {}

void LoRaPacket::changePayloadSize(size_t new_val) {
    _payloadSize = new_val;
}

bool LoRaPacket::validatePacket(const std::vector<uint8_t>& raw) {
    if (raw.size() < 8) return false;
    size_t raw_size = raw.size() - 1;
    if (raw[raw_size] != END_BYTE) return false;
    uint16_t crc = computeCRC(std::vector<uint8_t>(raw.begin(), raw.end() - 3));
    uint16_t reccievedCRC = (static_cast<uint16_t>(raw[raw.size() - 2]) << 8) | raw[raw.size() - 1];
    return crc == reccievedCRC;
}

void LoRaPacket::deserialize(const std::vector<uint8_t>& raw) {
    if (!validatePacket(raw)) {
        throw std::runtime_error("Invalid packet");
    }
    size_t raw_size = raw.size();
    _seqNum = (static_cast<uint16_t>(raw[raw_size - 4]) << 8) | raw[raw_size - 3];
    _msgType = raw[raw_size - 6];
    _options.raw = raw[raw_size - 7];
    for (size_t i = 0; i < raw_size - 8; ++i) {
        _payload.push_back(raw[i]);
    }
    _payloadSize = raw_size - 8;
    _rawPacket = raw;
}

std::vector<uint8_t> LoRaPacket::serialize() {
    std::vector<uint8_t> raw;
    raw.insert(raw.end(), _payload.begin(), _payload.end());
    raw.push_back(_options.raw);
    raw.push_back(_msgType);
    raw.push_back(static_cast<uint8_t>(_seqNum >> 8));
    raw.push_back(static_cast<uint8_t>(_seqNum & 0xFF));
    uint16_t crc = computeCRC(raw);
    raw.push_back(static_cast<uint8_t>(crc >> 8));
    raw.push_back(static_cast<uint8_t>(crc & 0xFF));
    raw.push_back(END_BYTE);
    _rawPacket = raw;
    return raw;
}

uint16_t LoRaPacket::computeCRC(const std::vector<uint8_t>& raw) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < raw.size(); i++) {
        crc ^= raw[i];
        for (size_t j = 0; j < 8; j++) {
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

void LoRaPacket::checkPayloadSize(const std::vector<uint8_t>& payload) {
    if (payload.size() > MAX_PAYLOAD) {
        throw std::runtime_error("Payload size exceeds maximum limit");
    }
}

void LoRaPacket::setPayload(const std::vector<uint8_t>& payload) {
    checkPayloadSize(payload);
    _payload.insert(_payload.end(), payload.begin(), payload.end());
    _payloadSize += payload.size();
}
