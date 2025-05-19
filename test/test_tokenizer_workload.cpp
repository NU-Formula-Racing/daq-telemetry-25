#include <builder/token_reader.hpp>
#include <builder/tokenizer.hpp>
#include <cmath>

#include "test.hpp"

using can::IdentifierPool;
using can::IdentifierPoolHandle;
using can::MockTokenReader;
using can::Tokenizer;
using can::TokenType;

// Test mixed global option with identifiers and integer
void test_Tokenizer_GlobalOption() {
    MockTokenReader reader("!! logPeriodMs 50");
    Tokenizer tok(reader);
    TEST_ASSERT(tok.start());

    auto t1 = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_OPTION_PREFIX, t1.type);

    auto t2 = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_IDENTIFIER, t2.type);
    // Check that the identifier is "logPeriodMs"
    const char* txt = IdentifierPool::instance().get(t2.data.idHandle);
    TEST_ASSERT_EQUAL_STRING("logPeriodMs", txt);

    auto t3 = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_INT, t3.type);
    TEST_ASSERT_EQUAL_INT(50, t3.data.intValue);

    TEST_ASSERT_FALSE(tok.next());
    tok.end();
}

// Test board line: > BoardName description words
void test_Tokenizer_BoardLine() {
    MockTokenReader reader("> MOTOR_CTRL Main control board of the vehicle");
    Tokenizer tok(reader);
    TEST_ASSERT(tok.start());

    auto p = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_BOARD_PREFIX, p.type);

    auto id = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_IDENTIFIER, id.type);
    TEST_ASSERT_EQUAL_STRING("MOTOR_CTRL", IdentifierPool::instance().get(id.data.idHandle));

    // subsequent free-text words should be identifiers too
    auto w1 = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_IDENTIFIER, w1.type);
    TEST_ASSERT_EQUAL_STRING("Main", IdentifierPool::instance().get(w1.data.idHandle));
    auto w2 = tok.next().value();
    TEST_ASSERT_EQUAL_STRING("control", IdentifierPool::instance().get(w2.data.idHandle));
    auto w3 = tok.next().value();
    TEST_ASSERT_EQUAL_STRING("board", IdentifierPool::instance().get(w3.data.idHandle));
    auto w4 = tok.next().value();
    TEST_ASSERT_EQUAL_STRING("of", IdentifierPool::instance().get(w4.data.idHandle));
    auto w5 = tok.next().value();
    TEST_ASSERT_EQUAL_STRING("the", IdentifierPool::instance().get(w5.data.idHandle));
    auto w6 = tok.next().value();
    TEST_ASSERT_EQUAL_STRING("vehicle", IdentifierPool::instance().get(w6.data.idHandle));

    TEST_ASSERT_FALSE(tok.next());
    tok.end();
}

// Test message line: >> NAME 0x1FF 8
void test_Tokenizer_MessageLine() {
    MockTokenReader reader(">> STATUS 0x1FF 8");
    Tokenizer tok(reader);
    TEST_ASSERT(tok.start());

    auto p = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_MESSAGE_PREFIX, p.type);

    auto name = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_IDENTIFIER, name.type);
    TEST_ASSERT_EQUAL_STRING("STATUS", IdentifierPool::instance().get(name.data.idHandle));

    auto mid = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_HEX_INT, mid.type);
    TEST_ASSERT_EQUAL_UINT(0x1FF, mid.data.uintValue);

    auto size = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_INT, size.type);
    TEST_ASSERT_EQUAL_INT(8, size.data.intValue);

    TEST_ASSERT_FALSE(tok.next());
    tok.end();
}

// Test signal line: >>> SIG uint16 0 16 1 0 little
void test_Tokenizer_SignalLine() {
    MockTokenReader reader(">>> SIG uint16 0 16 1 0 little");
    Tokenizer tok(reader);
    TEST_ASSERT(tok.start());

    // Prefix token
    auto p = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_SIGNAL_PREFIX, p.type);

    // SIG (identifier)
    auto sig = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_IDENTIFIER, sig.type);
    TEST_ASSERT_EQUAL_STRING("SIG", IdentifierPool::instance().get(sig.data.idHandle));

    // uint16 (identifier)
    auto dt = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_IDENTIFIER, dt.type);
    TEST_ASSERT_EQUAL_STRING("uint16", IdentifierPool::instance().get(dt.data.idHandle));

    // Following fields are all integers (0,16,1,0)
    int expectedInts[] = {0, 16, 1, 0};
    for (int i = 0; i < 4; ++i) {
        auto tk = tok.next().value();
        TEST_ASSERT_EQUAL_INT(TokenType::TT_INT, tk.type);
        TEST_ASSERT_EQUAL_INT(expectedInts[i], tk.data.intValue);
    }

    // 'little' is an identifier
    auto endi = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_IDENTIFIER, endi.type);
    TEST_ASSERT_EQUAL_STRING("little", IdentifierPool::instance().get(endi.data.idHandle));

    // No more tokens
    TEST_ASSERT_FALSE(tok.next());
    tok.end();
};

TEST_FUNC(test_Tokenizer_GlobalOption);
TEST_FUNC(test_Tokenizer_BoardLine);
TEST_FUNC(test_Tokenizer_MessageLine);
TEST_FUNC(test_Tokenizer_SignalLine);
