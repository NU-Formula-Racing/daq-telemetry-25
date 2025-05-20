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
    if (!res) return false;
    outOpts = res.value();
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


TEST_FUNC(test_TelemBuilder_Simple);
TEST_FUNC(test_TelemBuilder_OptionOverride);
TEST_FUNC(test_TelemBuilder_SignEndianOverride);
