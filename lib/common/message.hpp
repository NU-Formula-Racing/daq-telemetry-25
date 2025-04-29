#ifndef LORA_PACKET_HPP
#define LORA_PACKET_HPP

#include <cstdint>
#include <stdexcept>
#include <vector>

union OptionsByte {
    uint8_t raw;
    struct {
        uint8_t needsAck : 1;
        uint8_t isAck : 1;
        uint8_t reserved : 6;
    };
};

class LoRaPacket {
   public:
    static constexpr uint8_t END_BYTE = 0xA1;   // Magic byte to check if the packet is indeed ours, in the end
    static constexpr size_t MAX_PAYLOAD = 248;  // Hard limit, if we changed the MAX_PAYLOAD
    // we would have packets bigger than 255 (LoRa max) due to the headers

    enum : uint8_t {
        OPT_NEEDS_ACK = 1 << 0,
        OPT_IS_ACK = 1 << 1
    };

    LoRaPacket();
    LoRaPacket(uint8_t msgType, uint8_t options, uint16_t seqNum, std::vector<uint8_t> payload);
    
    /**------------------------------------------------------------------------
     *                           PACKET SERIALIZATIOn
     *------------------------------------------------------------------------**/

    static bool validatePacket(const std::vector<uint8_t>& raw);
    void deserialize(const std::vector<uint8_t>& raw);
    std::vector<uint8_t> serialize();

    /**------------------------------------------------------------------------
     *                           GETTERS
     *------------------------------------------------------------------------**/
    uint8_t messageType() const noexcept { return _msgType; }
    bool needsAck() const noexcept { return _options.needsAck == 1; }
    bool isAck() const noexcept { return _options.isAck == 1; }
    uint16_t sequenceNumber() const noexcept { return _seqNum; }
    const std::vector<uint8_t>& payload() const noexcept { return _payload; }

    /**------------------------------------------------------------------------
     *                           SETTERS
     *------------------------------------------------------------------------**/
    void changePayloadSize(size_t newVal);
    void setPayload(const std::vector<uint8_t>& payload);
    void setOptions(uint8_t options) { _options.raw = options; }
    void setMessageType(uint8_t msg_type) { _msgType = msg_type; }
    void setSequenceNumber(uint16_t seq_num) { _seqNum = seq_num; }

   private:
    uint8_t _msgType;
    OptionsByte _options;
    uint16_t _seqNum;
    size_t _payloadSize;

    // TODO: investigate better methods of storing packet data to be more friendly for embedded
    std::vector<uint8_t> _payload;
    std::vector<uint8_t> _rawPacket;

    static uint16_t computeCRC(const std::vector<uint8_t>& data);
    static void checkPayloadSize(const std::vector<uint8_t>& payload);
};

#endif
