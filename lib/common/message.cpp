//is copilot working? autocomplete this if it is. guess it's not. 

#include "message.hpp"

LoRaPacket::LoRaPacket(uint8_t msg_type,
                       uint8_t options,
                       uint16_t seq_num,
                       std::vector<uint8_t> payload)
    : msg_type_(msg_type),
      options_{.raw = options},
      seq_num_(seq_num),
      payload_size_(payload.size()),
      payload_(std::move(payload)) {}

LoRaPacket::LoRaPacket()
    : msg_type_(0),
      options_{.raw = 0},
      seq_num_(0),
      payload_size_(0),
      payload_() {}

void LoRaPacket::change_payload_size(size_t new_val) {
    payload_size_ = new_val;
}

bool LoRaPacket::validate_packet(const std::vector<uint8_t>& raw) {
    if (raw.size() < 8) return false;
    size_t raw_size = raw.size()-1;
    if (raw[raw_size] != END_BYTE) return false; 
    uint16_t crc = compute_crc(std::vector<uint8_t>(raw.begin(), raw.end() - 3));
    uint16_t received_crc = (static_cast<uint16_t>(raw[raw.size() - 2]) << 8) | raw[raw.size() - 1];
    return crc == received_crc;
}

void LoRaPacket::deserialize(const std::vector<uint8_t>& raw) {
    if (!validate_packet(raw)) {
        throw std::runtime_error("Invalid packet");
    }
    size_t raw_size = raw.size();
    seq_num_ = (static_cast<uint16_t>(raw[raw_size-4]) << 8) | raw[raw_size-3];
    msg_type_ = raw[raw_size-6];
    options_.raw = raw[raw_size-7];
    for (size_t i = 0; i < raw_size - 8; ++i) {
        payload_.push_back(raw[i]);
    }
    payload_size_ = raw_size - 8;
    raw_packet_ = raw;
}

std::vector<uint8_t> LoRaPacket::serialize() {
    std::vector<uint8_t> raw;
    raw.insert(raw.end(), payload_.begin(), payload_.end());
    raw.push_back(options_.raw);
    raw.push_back(msg_type_);
    raw.push_back(static_cast<uint8_t>(seq_num_ >> 8));
    raw.push_back(static_cast<uint8_t>(seq_num_ & 0xFF));
    uint16_t crc = compute_crc(raw);
    raw.push_back(static_cast<uint8_t>(crc >> 8));
    raw.push_back(static_cast<uint8_t>(crc & 0xFF));
    raw.push_back(END_BYTE);
    raw_packet_ = raw;
    return raw;
    
}

uint16_t LoRaPacket::compute_crc(const std::vector<uint8_t>& raw) {
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

void LoRaPacket::check_payload_size(const std::vector<uint8_t>& payload) {
    if (payload.size() > MAX_PAYLOAD) {
        throw std::runtime_error("Payload size exceeds maximum limit");
    }
}

void LoRaPacket::set_payload(const std::vector<uint8_t>& payload) {
    check_payload_size(payload);
    payload_.insert(payload_.end(), payload.begin(), payload.end());
    payload_size_ += payload.size();
}

