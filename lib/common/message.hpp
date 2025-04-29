#ifndef LORA_PACKET_HPP
#define LORA_PACKET_HPP

#include <cstdint>
#include <vector>
#include <stdexcept>

union OptionsByte {
    uint8_t raw;
    struct {
        uint8_t needs_ack:1;
        uint8_t is_ack:1;
        uint8_t reserved:6;
    };
};

class LoRaPacket {
public:
    static constexpr uint8_t END_BYTE    = 0xA1; // Magic byte to check if the packet is indeed ours, in the end
    static constexpr size_t  MAX_PAYLOAD = 248; // Hard limit, if we changed the MAX_PAYLOAD
    // we would have packets bigger than 255 (LoRa max) due to the headers

    enum  : uint8_t {
        OPT_NEEDS_ACK = 1 << 0,
        OPT_IS_ACK    = 1 << 1
    };


    LoRaPacket();
    LoRaPacket(uint8_t msg_type,
               uint8_t options,
               uint16_t seq_num,
               std::vector<uint8_t> payload);

    void deserialize(const std::vector<uint8_t>& raw);
    std::vector<uint8_t> serialize();
    uint8_t message_type() const noexcept { return msg_type_; }
    bool needs_ack() const noexcept { return options_.needs_ack == 1;}
    bool is_ack() const noexcept { return options_.is_ack == 1; }
    uint16_t sequence_number() const noexcept { return seq_num_; }
    const std::vector<uint8_t>& payload() const noexcept { return payload_; }
    void change_payload_size(size_t new_val);
    static bool validate_packet(const std::vector<uint8_t>& raw);
    void set_payload(const std::vector<uint8_t>& payload);
    void set_options(uint8_t options) { options_.raw = options; }
    void set_message_type(uint8_t msg_type) { msg_type_ = msg_type; }
    void set_sequence_number(uint16_t seq_num) { seq_num_ = seq_num; }

private:
    uint8_t msg_type_;
    OptionsByte options_;
    uint16_t seq_num_;
    size_t payload_size_;
    std::vector<uint8_t> payload_;
    std::vector<uint8_t> raw_packet_;

    static uint16_t compute_crc(const std::vector<uint8_t>& data);
    static void check_payload_size(const std::vector<uint8_t>& payload);
};

#endif 
