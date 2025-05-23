#include <builder/telem_builder.hpp>
#include <builder/token_reader.hpp>
#include <builder/tokenizer.hpp>
#include <can.hpp>
#include <cmath>
#include <cstring>

#include "test.hpp"

using can::CANBaudRate;
using can::CANBus;
using can::CANDriver;
using can::CANMessage;
using can::CANSignal;
using can::MockTokenReader;
using can::TelemBuilder;
using can::TelemetryOptions;
using can::Tokenizer;
using common::Result;

// A no-op driver for testing
class TestDriver : public CANDriver {};

// Helper: build bus and options from config
static bool buildBus(const char* cfg, TelemetryOptions& outOpts, CANBus& bus) {
    MockTokenReader reader(cfg);
    Tokenizer tok(reader);

    if (!tok.start()) return false;

    TelemBuilder builder(tok);
    auto res = builder.build(bus);

    if (res.isError()) {
        std::cout << res.error();
        return false;
    }

    outOpts = res.value();

    bus.printBus(std::cout);
    return true;
}

// Test: basic parsing of one board, one message, one signal
void test_TelemBuilder_Simple() {
    const char* cfg =
        "!! logPeriodMs 123\n"
        "> BOARD1\n"
        ">> MSG_A 0x100 2\n"
        ">>> SIG1 uint8 0 8 2 1\n";

    TelemetryOptions opts;
    TestDriver drv;
    CANBus bus(drv, CANBaudRate::CBR_125KBPS);
    bool ok = buildBus(cfg, opts, bus);
    TEST_ASSERT(ok);

    // Check overridden option
    TEST_ASSERT_EQUAL_INT(123, opts.logPeriodMs);

    // Inspect messages directly
    const auto& msgs = bus.getMessages();
    TEST_ASSERT_EQUAL_INT(1, msgs.size());
    auto it = msgs.find(0x100);
    TEST_ASSERT(it != msgs.end());
    CANMessage& msg = *it->second;

    TEST_ASSERT_EQUAL_UINT(2, msg.length);
    TEST_ASSERT_EQUAL_INT(1, msg.signals.size());

    const CANSignal& sig = msg.signals[0];
    TEST_ASSERT_EQUAL_UINT(8, sig.handle.size);
    TEST_ASSERT_FLOAT_WITHIN(1e-6, 2.0, sig.factor);
    TEST_ASSERT_FLOAT_WITHIN(1e-6, 1.0, sig.offset);
}

// Test: option override
void test_TelemBuilder_OptionOverride() {
    const char* cfg =
        "!! logPeriodMs 10\n"
        "!! logPeriodMs 20\n"
        "> B\n"
        ">> M 0x200 1\n"
        ">>> S bool 0 1 1 0\n";

    TelemetryOptions opts;
    TestDriver drv;
    CANBus bus(drv, CANBaudRate::CBR_125KBPS);
    bool ok = buildBus(cfg, opts, bus);
    TEST_ASSERT(ok);
    TEST_ASSERT_EQUAL_INT(20, opts.logPeriodMs);
}

// Test: signedness and endianness override
void test_TelemBuilder_SignEndianOverride() {
    const char* cfg =
        "> B2\n"
        ">> M2 0x2A0 4\n"
        ">>> S2 int16 8 16 0.5 -1 signed big\n";

    TelemetryOptions opts;
    TestDriver drv;
    CANBus bus(drv, CANBaudRate::CBR_125KBPS);
    bool ok = buildBus(cfg, opts, bus);
    TEST_ASSERT(ok);

    const auto& msgs = bus.getMessages();
    auto it = msgs.find(0x2A0);
    TEST_ASSERT(it != msgs.end());
    const CANMessage& msg = *it->second;
    TEST_ASSERT_EQUAL_UINT(4, msg.length);
    TEST_ASSERT_EQUAL_INT(1, msg.signals.size());

    const CANSignal& sig = msg.signals[0];
    TEST_ASSERT_TRUE(sig.isSigned);
    TEST_ASSERT_EQUAL_INT(can::MSG_BIG_ENDIAN, sig.endianness);
}

// Test: multiple boards yields error
void test_TelemBuilder_MultipleBoards() {
    const char* cfg =
        "> B1\n"
        "> B2\n"
        ">> M1 0x100 1\n"
        ">>> S1 uint8 0 8 1 0\n";

    TelemetryOptions opts;
    TestDriver drv;
    CANBus bus(drv, CANBaudRate::CBR_125KBPS);
    TEST_ASSERT_FALSE(buildBus(cfg, opts, bus));
}

// Test: duplicate message IDs within the same board
void test_TelemBuilder_DuplicateMessageID() {
    const char* cfg =
        "> B\n"
        ">> M1 0x100 1\n"
        ">>> S1 uint8 0 8 1 0\n"
        ">> M2 0x100 2\n"
        ">>> S2 uint16 0 16 1 0\n";

    TelemetryOptions opts;
    TestDriver drv;
    CANBus bus(drv, CANBaudRate::CBR_125KBPS);
    TEST_ASSERT_FALSE(buildBus(cfg, opts, bus));
}

// Test: overlapping signals within a message
void test_TelemBuilder_SignalOverlap() {
    const char* cfg =
        "> B\n"
        ">> M 0x100 2\n"
        ">>> S1 uint8 0 8 1 0\n"
        ">>> S2 uint8 4 8 2 0\n";  // overlaps bits 4-7

    TelemetryOptions opts;
    TestDriver drv;
    CANBus bus(drv, CANBaudRate::CBR_125KBPS);
    TEST_ASSERT_FALSE(buildBus(cfg, opts, bus));
}

// Test: signal bit range exceeds message payload
void test_TelemBuilder_SignalOutOfRange() {
    const char* cfg =
        "> B\n"
        ">> M 0x100 1\n"
        ">>> S uint8 0 16 1 0\n";  // length 16 > 8 bits

    TelemetryOptions opts;
    TestDriver drv;
    CANBus bus(drv, CANBaudRate::CBR_125KBPS);
    TEST_ASSERT_FALSE(buildBus(cfg, opts, bus));
}

// Test: message defined without board
void test_TelemBuilder_MessageWithoutBoard() {
    const char* cfg =
        ">> M 0x100 1\n"
        ">>> S uint8 0 8 1 0\n";

    TelemetryOptions opts;
    TestDriver drv;
    CANBus bus(drv, CANBaudRate::CBR_125KBPS);
    TEST_ASSERT_FALSE(buildBus(cfg, opts, bus));
}

// Test: signal defined without message context
void test_TelemBuilder_SignalWithoutMessage() {
    const char* cfg =
        "> B\n"
        ">>> S uint8 0 8 1 0\n";

    TelemetryOptions opts;
    TestDriver drv;
    CANBus bus(drv, CANBaudRate::CBR_125KBPS);
    TEST_ASSERT_FALSE(buildBus(cfg, opts, bus));
}

// Test: complex valid config with multiple boards, messages, and signals
void test_TelemBuilder_ComplexValid() {
    const char* cfg =
        "!! logPeriodMs 25"
        "> B1"
        ">> M1A 0x101 4"
        ">>> S1A1 uint8 0 8 1 0"
        ">>> S1A2 uint16 8 16 0.5 1"
        ">> M1B 0x102 2"
        ">>> S1B1 bool 0 1 1 0"
        "> B2"
        ">> M2 0x200 3"
        ">>> S2A float 0 32 0.1 0"
        ">>> S2B int8 32 8 2 2 signed";

    TelemetryOptions opts;
    TestDriver drv;
    CANBus bus(drv, CANBaudRate::CBR_500KBPS);
    bool ok = buildBus(cfg, opts, bus);
    TEST_ASSERT(ok);
    // Expect three messages total
    const auto& msgs = bus.getMessages();
    TEST_ASSERT_EQUAL_INT(3, msgs.size());
}

// Test: signal before any board context (invalid)
void test_TelemBuilder_SignalBeforeBoard() {
    const char* cfg =
        ">>> S uint8 0 8 1 0"
        "> B"
        ">> M 0x100 1"
        ">>> S1 uint8 0 8 1 0";

    TelemetryOptions opts;
    TestDriver drv;
    CANBus bus(drv, CANBaudRate::CBR_125KBPS);
    TEST_ASSERT_FALSE(buildBus(cfg, opts, bus));
}

// Test: message before any board (invalid)
void test_TelemBuilder_MessageBeforeBoard() {
    const char* cfg =
        ">> M 0x100 1"
        ">>> S uint8 0 8 1 0"
        "> B";

    TelemetryOptions opts;
    TestDriver drv;
    CANBus bus(drv, CANBaudRate::CBR_125KBPS);
    TEST_ASSERT_FALSE(buildBus(cfg, opts, bus));
}

// Test: board declared after a message (invalid hierarchy)
void test_TelemBuilder_BoardAfterMessage() {
    const char* cfg =
        "> B1"
        ">> M1 0x100 1"
        "> B2"
        ">>> S1 uint8 0 8 1 0";

    TelemetryOptions opts;
    TestDriver drv;
    CANBus bus(drv, CANBaudRate::CBR_125KBPS);
    TEST_ASSERT_FALSE(buildBus(cfg, opts, bus));
}

// Test: duplicate signal names within the same message (invalid)
void test_TelemBuilder_DuplicateSignalNames() {
    const char* cfg =
        "> B"
        ">> M 0x100 2"
        ">>> S1 uint8 0 8 1 0"
        ">>> S1 uint8 8 8 1 0";

    TelemetryOptions opts;
    TestDriver drv;
    CANBus bus(drv, CANBaudRate::CBR_125KBPS);
    TEST_ASSERT_FALSE(buildBus(cfg, opts, bus));
}

// Test: duplicate board names (invalid)
void test_TelemBuilder_DuplicateBoardNames() {
    const char* cfg =
        "> B"
        "> B"
        ">> M 0x100 1"
        ">>> S uint8 0 8 1 0";

    TelemetryOptions opts;
    TestDriver drv;
    CANBus bus(drv, CANBaudRate::CBR_125KBPS);
    TEST_ASSERT_FALSE(buildBus(cfg, opts, bus));
}

TEST_FUNC(test_TelemBuilder_Simple);
TEST_FUNC(test_TelemBuilder_OptionOverride);
TEST_FUNC(test_TelemBuilder_SignEndianOverride);
// Register tests
TEST_FUNC(test_TelemBuilder_MultipleBoards);
TEST_FUNC(test_TelemBuilder_DuplicateMessageID);
TEST_FUNC(test_TelemBuilder_SignalOverlap);
TEST_FUNC(test_TelemBuilder_SignalOutOfRange);
TEST_FUNC(test_TelemBuilder_MessageWithoutBoard);
TEST_FUNC(test_TelemBuilder_SignalWithoutMessage);
TEST_FUNC(test_TelemBuilder_ComplexValid);
TEST_FUNC(test_TelemBuilder_SignalBeforeBoard);
TEST_FUNC(test_TelemBuilder_MessageBeforeBoard);
TEST_FUNC(test_TelemBuilder_BoardAfterMessage);
TEST_FUNC(test_TelemBuilder_DuplicateSignalNames);
TEST_FUNC(test_TelemBuilder_DuplicateBoardNames);