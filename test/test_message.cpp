#include "test.hpp"
#include <message.hpp>
#include <vector>

namespace common {

// Test basic initialization of LoRaPacket
void test_MessageInit() {
    LoRaPacket packet;
    TEST_ASSERT_EQUAL_UINT8(0, packet.messageType());
    TEST_ASSERT_FALSE(packet.needsAck());
    TEST_ASSERT_FALSE(packet.isAck());
    TEST_ASSERT_EQUAL_UINT16(0, packet.sequenceNumber());
    TEST_ASSERT_EQUAL_UINT(0, packet.payloadSize());
    TEST_ASSERT_EQUAL_UINT(0, packet.payload().size());
}

// Test initialization with parameters
void test_MessageInitWithParams() {
    std::vector<uint8_t> payload = {0x01, 0x02, 0x03};
    LoRaPacket packet(0xAA, LoRaPacket::OPT_NEEDS_ACK, 0x1234, payload);
    
    TEST_ASSERT_EQUAL_UINT8(0xAA, packet.messageType());
    TEST_ASSERT_TRUE(packet.needsAck());
    TEST_ASSERT_FALSE(packet.isAck());
    TEST_ASSERT_EQUAL_UINT16(0x1234, packet.sequenceNumber());
    TEST_ASSERT_EQUAL_UINT(3, packet.payloadSize());
    
    auto retrieved_payload = packet.payload();
    TEST_ASSERT_EQUAL_UINT(3, retrieved_payload.size());
    TEST_ASSERT_EQUAL_UINT8(0x01, retrieved_payload[0]);
    TEST_ASSERT_EQUAL_UINT8(0x02, retrieved_payload[1]);
    TEST_ASSERT_EQUAL_UINT8(0x03, retrieved_payload[2]);
}

// Test serialization and deserialization
void test_MessageSerializeDeserialize() {
    std::vector<uint8_t> payload = {0x01, 0x02, 0x03};
    LoRaPacket original(0xAA, LoRaPacket::OPT_NEEDS_ACK, 0x1234, payload);
    
    const std::vector<uint8_t>& serialized = original.serialize();
    TEST_ASSERT_TRUE(LoRaPacket::validatePacket(serialized));
    
    LoRaPacket deserialized;
    auto result = deserialized.deserialize(serialized);
    TEST_ASSERT_TRUE(result.isSome());
    
    TEST_ASSERT_EQUAL_UINT8(original.messageType(), deserialized.messageType());
    TEST_ASSERT_EQUAL_UINT8(original.needsAck(), deserialized.needsAck());
    TEST_ASSERT_EQUAL_UINT8(original.isAck(), deserialized.isAck());
    TEST_ASSERT_EQUAL_UINT16(original.sequenceNumber(), deserialized.sequenceNumber());
    TEST_ASSERT_EQUAL_UINT(original.payloadSize(), deserialized.payloadSize());
    
    auto original_payload = original.payload();
    auto deserialized_payload = deserialized.payload();
    TEST_ASSERT_EQUAL_UINT(original_payload.size(), deserialized_payload.size());
    for (size_t i = 0; i < original_payload.size(); i++) {
        TEST_ASSERT_EQUAL_UINT8(original_payload[i], deserialized_payload[i]);
    }
}

// Test payload size limits
void test_MessagePayloadSize() {
    std::vector<uint8_t> payload(LoRaPacket::MAX_PAYLOAD, 0xAA);
    LoRaPacket packet(0xAA, 0, 0x1234, payload);
    TEST_ASSERT_EQUAL_UINT(LoRaPacket::MAX_PAYLOAD, packet.payloadSize());
    
    // Test setting payload that exceeds maximum size
    std::vector<uint8_t> too_large_payload(LoRaPacket::MAX_PAYLOAD + 1, 0xAA);
    auto result = packet.setPayload(too_large_payload);
    TEST_ASSERT_TRUE(result.isNone());
}

// Test invalid packet validation
void test_MessageInvalidPacket() {
    // Test packet too short
    std::vector<uint8_t> invalid_packet = {0x01, 0x02, 0x03};
    TEST_ASSERT_FALSE(LoRaPacket::validatePacket(invalid_packet));
    
    // Test with wrong end byte
    std::vector<uint8_t> wrong_end = {
        0x01, 0x02, 0x03,  // payload
        0x00, 0x00,        // options, msgType
        0x00, 0x00,        // seqNum
        0x00, 0x00,        // CRC
        0x00               // Wrong END_BYTE
    };
    TEST_ASSERT_FALSE(LoRaPacket::validatePacket(wrong_end));
    
    // Test with wrong CRC
    std::vector<uint8_t> wrong_crc = {
        0x01, 0x02, 0x03,  // payload
        0x00, 0x00,        // options, msgType
        0x00, 0x00,        // seqNum
        0x00, 0x00,        // Wrong CRC
        LoRaPacket::END_BYTE
    };
    TEST_ASSERT_FALSE(LoRaPacket::validatePacket(wrong_crc));
    
    // Test deserializing invalid packet
    LoRaPacket packet;
    auto result = packet.deserialize(invalid_packet);
    TEST_ASSERT_TRUE(result.isNone());
}

// Test setters and verify they update the internal state correctly
void test_MessageSetters() {
    LoRaPacket packet;
    
    packet.setMessageType(0xBB);
    TEST_ASSERT_EQUAL_UINT8(0xBB, packet.messageType());
    
    packet.setOptions(LoRaPacket::OPT_IS_ACK);
    TEST_ASSERT_TRUE(packet.isAck());
    TEST_ASSERT_FALSE(packet.needsAck());
    
    packet.setSequenceNumber(0x5678);
    TEST_ASSERT_EQUAL_UINT16(0x5678, packet.sequenceNumber());
    
    std::vector<uint8_t> new_payload = {0x04, 0x05, 0x06};
    auto result = packet.setPayload(new_payload);
    TEST_ASSERT_TRUE(result.isSome());
    
    auto retrieved_payload = packet.payload();
    TEST_ASSERT_EQUAL_UINT(3, retrieved_payload.size());
    TEST_ASSERT_EQUAL_UINT8(0x04, retrieved_payload[0]);
    TEST_ASSERT_EQUAL_UINT8(0x05, retrieved_payload[1]);
    TEST_ASSERT_EQUAL_UINT8(0x06, retrieved_payload[2]);
    
    // Verify that CRC is updated after modifications
    const std::vector<uint8_t>& serialized = packet.serialize();
    TEST_ASSERT_TRUE(LoRaPacket::validatePacket(serialized));
}

TEST_FUNC(test_MessageInit);
TEST_FUNC(test_MessageInitWithParams);
TEST_FUNC(test_MessageSerializeDeserialize);
TEST_FUNC(test_MessagePayloadSize);
TEST_FUNC(test_MessageInvalidPacket);
TEST_FUNC(test_MessageSetters);

}  // namespace common 