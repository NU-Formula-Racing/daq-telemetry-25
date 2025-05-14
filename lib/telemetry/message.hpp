#ifndef LORA_PACKET_HPP
#define LORA_PACKET_HPP

#include <cstdint>
#include <vector>
#include <common/option.hpp>

namespace common {

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

    // Packet format:
    // [payload][options][msgType][seqNum_H][seqNum_L][crc_H][crc_L][END_BYTE]
    static constexpr size_t HEADER_SIZE = 7;  // options + msgType + seqNum(2) + crc(2) + END_BYTE

    enum : uint8_t {
        OPT_NEEDS_ACK = 1 << 0,
        OPT_IS_ACK = 1 << 1
    };

    LoRaPacket();
    LoRaPacket(uint8_t msgType, uint8_t options, uint16_t seqNum, const std::vector<uint8_t>& payload);
    
    /**------------------------------------------------------------------------
     *                           PACKET SERIALIZATIOn
     *------------------------------------------------------------------------**/

    static bool validatePacket(const std::vector<uint8_t>& raw);
    Option<void *> deserialize(const std::vector<uint8_t>& raw);
    const std::vector<uint8_t>& serialize();

    /**------------------------------------------------------------------------
     *                           GETTERS
     *------------------------------------------------------------------------**/
    uint8_t messageType() const noexcept;
    bool needsAck() const noexcept;
    bool isAck() const noexcept;
    uint16_t sequenceNumber() const noexcept;
    std::vector<uint8_t> payload() const noexcept;
    size_t payloadSize() const noexcept;

    /**------------------------------------------------------------------------
     *                           SETTERS
     *------------------------------------------------------------------------**/
    Option<void *> setPayload(const std::vector<uint8_t>& payload);
    void setOptions(uint8_t options);
    void setMessageType(uint8_t msg_type);
    void setSequenceNumber(uint16_t seq_num);

   private:
    std::vector<uint8_t> _rawPacket;
    bool _isDirty;  // Indicates if _rawPacket needs to be re-serialized

    static uint16_t computeCRC(const std::vector<uint8_t>& data);
    static Option<void *> checkPayloadSize(size_t size);
    void ensurePacketSize(size_t payloadSize);
    void updateCRC();
};

}  // namespace common

#endif
