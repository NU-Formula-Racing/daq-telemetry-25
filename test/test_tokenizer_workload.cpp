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

// Negative decimal integer
void test_Tokenizer_NegativeInt() {
    MockTokenReader reader("-123");
    Tokenizer tok(reader);
    TEST_ASSERT(tok.start());

    auto t = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_INT, t.type);
    TEST_ASSERT_EQUAL_INT(-123, t.data.intValue);

    TEST_ASSERT_FALSE(tok.next());
    tok.end();
}

// Negative floating‐point
void test_Tokenizer_NegativeFloat() {
    MockTokenReader reader("-3.5");
    Tokenizer tok(reader);
    TEST_ASSERT(tok.start());

    auto t = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_FLOAT, t.type);
    TEST_ASSERT(fabs(t.data.floatValue + 3.5) < 1e-6);

    TEST_ASSERT_FALSE(tok.next());
    tok.end();
}

// Upper‐ and lower‐case hex parsing
void test_Tokenizer_HexCaseInsensitivity() {
    MockTokenReader reader("0XFF 0xab");
    Tokenizer tok(reader);
    TEST_ASSERT(tok.start());

    auto h1 = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_HEX_INT, h1.type);
    TEST_ASSERT_EQUAL_UINT(0xFF, h1.data.uintValue);

    auto h2 = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_HEX_INT, h2.type);
    TEST_ASSERT_EQUAL_UINT(0xAB, h2.data.uintValue);

    TEST_ASSERT_FALSE(tok.next());
    tok.end();
}

// Scientific‐notation floats
void test_Tokenizer_SciFloat() {
    MockTokenReader reader("1e3 2E-2");
    Tokenizer tok(reader);
    TEST_ASSERT(tok.start());

    auto f1 = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_FLOAT, f1.type);
    TEST_ASSERT(fabs(f1.data.floatValue - 1000.0) < 1e-6);

    auto f2 = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_FLOAT, f2.type);
    TEST_ASSERT(fabs(f2.data.floatValue - 0.02) < 1e-6);

    TEST_ASSERT_FALSE(tok.next());
    tok.end();
}

// Alphanumeric identifier
void test_Tokenizer_AlphaNumIdentifier() {
    MockTokenReader reader("Signal123 _under_score1");
    Tokenizer tok(reader);
    TEST_ASSERT(tok.start());

    auto s1 = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_IDENTIFIER, s1.type);

    auto s2 = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_IDENTIFIER, s2.type);

    TEST_ASSERT_FALSE(tok.next());
    tok.end();
}

// Mixed whitespace and comments
void test_Tokenizer_WhitespaceAndComments() {
    MockTokenReader reader("   7\t\t8  #skip me\n   9 ");
    Tokenizer tok(reader);
    TEST_ASSERT(tok.start());

    auto n1 = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_INT, n1.type);
    TEST_ASSERT_EQUAL_INT(7, n1.data.intValue);

    auto n2 = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_INT, n2.type);
    TEST_ASSERT_EQUAL_INT(8, n2.data.intValue);

    // skip comment, then next word
    auto n3 = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_INT, n3.type);
    TEST_ASSERT_EQUAL_INT(9, n3.data.intValue);

    TEST_ASSERT_FALSE(tok.next());
    tok.end();
}



TEST_FUNC(test_Tokenizer_GlobalOption);
TEST_FUNC(test_Tokenizer_BoardLine);
TEST_FUNC(test_Tokenizer_MessageLine);
TEST_FUNC(test_Tokenizer_SignalLine);
TEST_FUNC(test_Tokenizer_NegativeInt);
TEST_FUNC(test_Tokenizer_NegativeFloat);
TEST_FUNC(test_Tokenizer_HexCaseInsensitivity);
TEST_FUNC(test_Tokenizer_SciFloat);
TEST_FUNC(test_Tokenizer_AlphaNumIdentifier);
TEST_FUNC(test_Tokenizer_WhitespaceAndComments);