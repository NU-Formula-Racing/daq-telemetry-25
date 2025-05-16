#include "test.hpp"
#include <builder/token_reader.hpp>
#include <cstring>

using can::MockTokenReader;

// Test behavior on empty content
void test_MockCR_Empty() {
    MockTokenReader reader("");
    TEST_ASSERT(reader.start());
    char buf[16];
    size_t len = 0;
    // No words available
    TEST_ASSERT_FALSE(reader.peekNextWord(sizeof(buf), buf, &len));
    // moveWord should fail
    TEST_ASSERT_FALSE(reader.moveWord());
    reader.end();
}

// Test a single word without surrounding whitespace
void test_MockCR_SingleWord() {
    MockTokenReader reader("hello");
    TEST_ASSERT(reader.start());
    char buf[8];
    size_t len = 0;
    // Peek should see "hello"
    TEST_ASSERT(reader.peekNextWord(sizeof(buf), buf, &len));
    TEST_ASSERT_EQUAL_UINT(5, len);
    TEST_ASSERT_EQUAL_STRING("hello", buf);
    // Re-peeking does not advance
    len = 0;
    TEST_ASSERT(reader.peekNextWord(sizeof(buf), buf, &len));
    TEST_ASSERT_EQUAL_UINT(5, len);
    // Move past the one word
    TEST_ASSERT(reader.moveWord());
    // Now no more words
    TEST_ASSERT_FALSE(reader.peekNextWord(sizeof(buf), buf, &len));
    reader.end();
}

// Test multiple words with various whitespace
void test_MockCR_MultipleWords() {
    MockTokenReader reader(" one\t two\nthree   four ");
    TEST_ASSERT(reader.start());
    char buf[8];
    size_t len = 0;

    TEST_ASSERT(reader.peekNextWord(sizeof(buf), buf, &len));
    TEST_ASSERT_EQUAL_UINT(3, len);
    TEST_ASSERT_EQUAL_STRING("one", buf);

    TEST_ASSERT(reader.moveWord());
    TEST_ASSERT(reader.peekNextWord(sizeof(buf), buf, &len));
    TEST_ASSERT_EQUAL_UINT(3, len);
    TEST_ASSERT_EQUAL_STRING("two", buf);

    // Skip two words at once: "three"
    TEST_ASSERT(reader.moveWord(2));
    TEST_ASSERT(reader.peekNextWord(sizeof(buf), buf, &len));
    TEST_ASSERT_EQUAL_UINT(4, len);
    TEST_ASSERT_EQUAL_STRING("four", buf);

    // Move past last word
    TEST_ASSERT(reader.moveWord());
    TEST_ASSERT_FALSE(reader.peekNextWord(sizeof(buf), buf, &len));
    reader.end();
}

// Test truncation when maxLength is smaller than word length
void test_MockCR_Truncation() {
    const char* longWord = "abcdefghijkl";
    MockTokenReader reader(longWord);
    TEST_ASSERT(reader.start());
    char buf[5];
    size_t len = 0;
    // maxLength = 5 => copyLen = 4, but len reports full 12
    TEST_ASSERT(reader.peekNextWord(sizeof(buf), buf, &len));
    TEST_ASSERT_EQUAL_UINT(12, len);
    TEST_ASSERT_EQUAL_STRING_LEN("abcd", buf, 4);
    reader.end();
}

// Test moveWord with stepSize > 1
void test_MockCR_MoveMultiple() {
    MockTokenReader reader("a b c d");
    TEST_ASSERT(reader.start());
    char buf[2];
    size_t len = 0;
    // Skip first two words
    TEST_ASSERT(reader.moveWord(2));
    TEST_ASSERT(reader.peekNextWord(sizeof(buf), buf, &len));
    TEST_ASSERT_EQUAL_UINT(1, len);
    TEST_ASSERT_EQUAL_STRING("c", buf);
    reader.end();
}

// Test that end() resets state and allows restart
void test_MockCR_EndAndRestart() {
    MockTokenReader reader("red green");
    TEST_ASSERT(reader.start());
    char buf[8];
    size_t len = 0;
    TEST_ASSERT(reader.peekNextWord(sizeof(buf), buf, &len));
    TEST_ASSERT_EQUAL_STRING("red", buf);
    reader.end();

    // After end, peek should fail until start() is called again
    TEST_ASSERT_FALSE(reader.peekNextWord(sizeof(buf), buf, &len));

    TEST_ASSERT(reader.start());
    TEST_ASSERT(reader.peekNextWord(sizeof(buf), buf, &len));
    TEST_ASSERT_EQUAL_STRING("red", buf);
    reader.end();
}

TEST_FUNC(test_MockCR_Empty);
TEST_FUNC(test_MockCR_SingleWord);
TEST_FUNC(test_MockCR_MultipleWords);
TEST_FUNC(test_MockCR_Truncation);
TEST_FUNC(test_MockCR_MoveMultiple);
TEST_FUNC(test_MockCR_EndAndRestart);
