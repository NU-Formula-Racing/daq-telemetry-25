#include "test.hpp"
#include <message.hpp>
#include <vector>

// Test basic initialization of LoRaPacket
void test_MessageInit() {
    LoRaPacket packet;
    TEST_ASSERT_EQUAL_UINT8(0, packet.messageType());
    TEST_ASSERT_FALSE(packet.needsAck());
    TEST_ASSERT_FALSE(packet.isAck());
    TEST_ASSERT_EQUAL_UINT16(0, packet.sequenceNumber());
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
    TEST_ASSERT_EQUAL_UINT(3, packet.payload().size());
    TEST_ASSERT_EQUAL_UINT8(0x01, packet.payload()[0]);
    TEST_ASSERT_EQUAL_UINT8(0x02, packet.payload()[1]);
    TEST_ASSERT_EQUAL_UINT8(0x03, packet.payload()[2]);
}

// Test serialization and deserialization
void test_MessageSerializeDeserialize() {
    std::vector<uint8_t> payload = {0x01, 0x02, 0x03};
    LoRaPacket original(0xAA, LoRaPacket::OPT_NEEDS_ACK, 0x1234, payload);
    
    std::vector<uint8_t> serialized = original.serialize();
    std::cout << "fatos\n";
    TEST_ASSERT_TRUE(LoRaPacket::validatePacket(serialized));
    
    LoRaPacket deserialized;
    deserialized.deserialize(serialized);
    TEST_ASSERT_EQUAL_UINT8(original.messageType(), deserialized.messageType());
    TEST_ASSERT_EQUAL_UINT8(original.needsAck(), deserialized.needsAck());
    TEST_ASSERT_EQUAL_UINT8(original.isAck(), deserialized.isAck());
    TEST_ASSERT_EQUAL_UINT16(original.sequenceNumber(), deserialized.sequenceNumber());
    TEST_ASSERT_EQUAL_UINT(original.payload().size(), deserialized.payload().size());
    for (size_t i = 0; i < original.payload().size(); i++) {
        TEST_ASSERT_EQUAL_UINT8(original.payload()[i], deserialized.payload()[i]);
    }
}

// Test payload size limits
void test_MessagePayloadSize() {
    std::vector<uint8_t> payload(LoRaPacket::MAX_PAYLOAD, 0xAA);
    LoRaPacket packet(0xAA, 0, 0x1234, payload);
    TEST_ASSERT_EQUAL_UINT(LoRaPacket::MAX_PAYLOAD, packet.payload().size());
    
    // Test setting payload that exceeds maximum size
    std::vector<uint8_t> tooLargePayload(LoRaPacket::MAX_PAYLOAD + 1, 0xAA);
    bool exceptionThrown = false;
    try {
        packet.setPayload(tooLargePayload);
    } catch (const std::runtime_error&) {
        exceptionThrown = true;
    }
    TEST_ASSERT_TRUE(exceptionThrown);
}

// Test invalid packet validation
void test_MessageInvalidPacket() {
    std::vector<uint8_t> invalidPacket = {0x01, 0x02, 0x03}; // Too short
    TEST_ASSERT_FALSE(LoRaPacket::validatePacket(invalidPacket));
    
    // Test with wrong end byte
    std::vector<uint8_t> wrongEnd = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00};
    TEST_ASSERT_FALSE(LoRaPacket::validatePacket(wrongEnd));
    
    // Test with wrong CRC
    std::vector<uint8_t> wrongCrc = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00, 0x00, 0x00, 0xA1};
    TEST_ASSERT_FALSE(LoRaPacket::validatePacket(wrongCrc));
}

// Test setters
void test_MessageSetters() {
    LoRaPacket packet;
    
    packet.setMessageType(0xBB);
    TEST_ASSERT_EQUAL_UINT8(0xBB, packet.messageType());
    
    packet.setOptions(LoRaPacket::OPT_IS_ACK);
    TEST_ASSERT_TRUE(packet.isAck());
    
    packet.setSequenceNumber(0x5678);
    TEST_ASSERT_EQUAL_UINT16(0x5678, packet.sequenceNumber());
    
    std::vector<uint8_t> newPayload = {0x04, 0x05, 0x06};
    packet.setPayload(newPayload);
    TEST_ASSERT_EQUAL_UINT(3, packet.payload().size());
    TEST_ASSERT_EQUAL_UINT8(0x04, packet.payload()[0]);
    TEST_ASSERT_EQUAL_UINT8(0x05, packet.payload()[1]);
    TEST_ASSERT_EQUAL_UINT8(0x06, packet.payload()[2]);
}

TEST_FUNC(test_MessageInit);
TEST_FUNC(test_MessageInitWithParams);
TEST_FUNC(test_MessageSerializeDeserialize);
TEST_FUNC(test_MessagePayloadSize);
TEST_FUNC(test_MessageInvalidPacket);
TEST_FUNC(test_MessageSetters); 