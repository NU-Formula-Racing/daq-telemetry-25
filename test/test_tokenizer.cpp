#include "test.hpp"
#include <builder/token_reader.hpp>
#include <builder/tokenizer.hpp>
#include <cmath>

using can::MockTokenReader;
using can::Tokenizer;
using can::TokenType;

// Test: empty input yields no tokens
void test_Tokenizer_Empty() {
    MockTokenReader reader("");
    Tokenizer tok(reader);
    TEST_ASSERT(tok.start());
    auto opt = tok.next();
    TEST_ASSERT_FALSE(opt);
    tok.end();
}

// Test recognition of all prefix tokens
void test_Tokenizer_Prefixes() {
    MockTokenReader reader("!! > >> >>> >>>>");
    Tokenizer tok(reader);
    TEST_ASSERT(tok.start());

    auto t1 = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_OPTION_PREFIX, t1.type);

    auto t2 = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_BOARD_PREFIX, t2.type);

    auto t3 = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_MESSAGE_PREFIX, t3.type);

    auto t4 = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_SIGNAL_PREFIX, t4.type);

    auto t5 = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_ENUM_PREFIX, t5.type);

    // no more tokens
    TEST_ASSERT_FALSE(tok.next());
    tok.end();
}

// Test hex and decimal integer parsing
void test_Tokenizer_HexAndInt() {
    MockTokenReader reader("0x1A 42");
    Tokenizer tok(reader);
    TEST_ASSERT(tok.start());

    auto h = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_HEX_INT, h.type);
    TEST_ASSERT_EQUAL_UINT(0x1A, h.data.uintValue);

    auto i = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_INT, i.type);
    TEST_ASSERT_EQUAL_INT(42, i.data.intValue);

    TEST_ASSERT_FALSE(tok.next());
    tok.end();
}

// Test floating-point parsing
void test_Tokenizer_Float() {
    MockTokenReader reader("3.14");
    Tokenizer tok(reader);
    TEST_ASSERT(tok.start());

    auto f = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_FLOAT, f.type);
    // allow small epsilon
    TEST_ASSERT(fabs(f.data.floatValue - 3.14) < 1e-6);

    TEST_ASSERT_FALSE(tok.next());
    tok.end();
}

// Test identifier fallback on parse failure
void test_Tokenizer_Identifier() {
    MockTokenReader reader("hello_world");
    Tokenizer tok(reader);
    TEST_ASSERT(tok.start());

    auto id = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_IDENTIFIER, id.type);
    
    TEST_ASSERT_FALSE(tok.next());
    tok.end();
}

// Test skipping of comments and blank words
void test_Tokenizer_SkipComments() {
    MockTokenReader reader("# this is a comment\nVALUE 100");
    Tokenizer tok(reader);
    TEST_ASSERT(tok.start());

    auto v = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_IDENTIFIER, v.type);

    auto n = tok.next().value();
    TEST_ASSERT_EQUAL_INT(TokenType::TT_INT, n.type);

    TEST_ASSERT_FALSE(tok.next());
    tok.end();
}

TEST_FUNC(test_Tokenizer_Empty);
TEST_FUNC(test_Tokenizer_Prefixes);
TEST_FUNC(test_Tokenizer_HexAndInt);
TEST_FUNC(test_Tokenizer_Float);
TEST_FUNC(test_Tokenizer_Identifier);
TEST_FUNC(test_Tokenizer_SkipComments);
