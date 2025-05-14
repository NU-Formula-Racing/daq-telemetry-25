#include "test.hpp"
#include <message_controller.hpp>
#include <vector>

using namespace common;
// Test basic initialization
void test_MessageControllerInit() {
    MessageController controller;
}

// Test sending messages
void test_MessageControllerSend() {
    MessageController controller;
    std::vector<uint8_t> payload = {0x01, 0x02, 0x03};
    
    controller.sendMessage(0xAA, payload, true);
    TEST_ASSERT_TRUE(controller.hasPendingAck());
    
    controller.sendMessage(0xBB, payload, false);
    TEST_ASSERT_TRUE(controller.hasPendingAck()); 
}

// Test ACK handling
void test_MessageControllerAck() {
    MessageController controller;
    std::vector<uint8_t> payload = {0x01, 0x02, 0x03};
    
    // Send a message that needs ACK
    controller.sendMessage(0xAA, payload, true);
    TEST_ASSERT_TRUE(controller.hasPendingAck());
    
    // Create an ACK packet with matching sequence number
    LoRaPacket ack;
    ack.setMessageType(0xAA);
    ack.setOptions(LoRaPacket::OPT_IS_ACK);
    ack.setSequenceNumber(0); // First sequence number
    
    // Process the ACK
    controller.processReceivedPacket(ack.serialize());
    TEST_ASSERT_FALSE(controller.hasPendingAck());
}

// Test message handling and callbacks
void test_MessageControllerHandleMessage() {
    MessageController controller;
    std::vector<uint8_t> payload = {0x01, 0x02, 0x03};
    
    // Create a test packet
    LoRaPacket packet(0xAA, 0, 0x1234, payload);
    
    controller.processReceivedPacket(packet.serialize());
}

// Test sequence number management
void test_MessageControllerSequence() {
    MessageController controller;
    std::vector<uint8_t> payload = {0x01};    
    for (uint16_t i = 0; i < 5; i++) {
        controller.sendMessage(0xAA, payload, true);
    }
}

// Test multiple pending ACKs
void test_MessageControllerMultipleAcks() {
    MessageController controller;
    std::vector<uint8_t> payload = {0x01};
    
    // Send multiple messages that need ACKs
    for (int i = 0; i < 3; i++) {
        controller.sendMessage(0xAA, payload, true);
    }
    
    // Verify all are pending
    TEST_ASSERT_TRUE(controller.hasPendingAck());
    
    // ACK them in order
    for (int i = 0; i < 3; i++) {
        LoRaPacket ack;
        ack.setMessageType(0xAA);
        ack.setOptions(LoRaPacket::OPT_IS_ACK);
        ack.setSequenceNumber(i);
        controller.processReceivedPacket(ack.serialize());
    }
    
    // Verify none are pending
    TEST_ASSERT_FALSE(controller.hasPendingAck());
}

TEST_FUNC(test_MessageControllerInit);
TEST_FUNC(test_MessageControllerSend);
TEST_FUNC(test_MessageControllerAck);
TEST_FUNC(test_MessageControllerHandleMessage);
TEST_FUNC(test_MessageControllerSequence);
TEST_FUNC(test_MessageControllerMultipleAcks); 