#include "test.hpp"
#include <bit_buffer.hpp>
#include <cstdlib>

// Test that initialization correctly reports the bit and byte sizes.
void test_BBInit() {
    const size_t bitCount = 16;
    BitBuffer bb(bitCount);
    TEST_ASSERT_EQUAL_UINT(bitCount, bb.bitSize());
    TEST_ASSERT_EQUAL_UINT((bitCount + 7) / 8, bb.byteSize());
}

// Test read/write when the data is exactly byte aligned.
void test_BBByteAlignedRW() {
    BitBuffer bb(8);  // 8-bit buffer (1 byte)
    uint8_t value = 0xAB;
    BitBufferHandle handle(8, 0);  // Write/read one full byte at offset 0.
    bb.write(handle, value);
    Option<uint8_t> result = bb.read<uint8_t>(handle);
    TEST_ASSERT(result.isSome());
    TEST_ASSERT_EQUAL_UINT8(value, result.value());
}

// Test read/write for a range that is within one byte (not an entire byte).
void test_BBWithinByteRW() {
    BitBuffer bb(8);  // One byte available.
    uint8_t value = 0x5;  // 4-bit value (e.g. 0101)
    BitBufferHandle handle(4, 0);  // Write only the lower 4 bits.
    bb.write(handle, value);
    Option<uint8_t> result = bb.read<uint8_t>(handle);
    TEST_ASSERT(result.isSome());
    // Compare only the lower 4 bits (mask with 0xF).
    TEST_ASSERT_EQUAL_UINT8(value, result.value() & 0xF);
}

// Test read/write where the data spans across two bytes.
void test_BBCrossByteRW() {
    BitBuffer bb(16);  // 16 bits = 2 bytes.
    uint16_t value = 0xABC; // 12-bit value (only lower 12 bits are relevant)
    BitBufferHandle handle(12, 4);  // Write 12 bits starting at bit offset 4.
    bb.write(handle, value);
    Option<uint16_t> result = bb.read<uint16_t>(handle);
    TEST_ASSERT(result.isSome());
    // Compare only lower 12 bits.
    TEST_ASSERT_EQUAL_UINT16(value & 0xFFF, result.value() & 0xFFF);
}

// Test that attempting to read or write outside the buffer’s allocated range fails.
void test_BBOutOfRangeRW() {
    BitBuffer bb(8);  // 8 bits total.
    uint8_t value = 0xFF;
    // Create a handle that starts at offset 4 with a size of 8 bits,
    // so (offset + size) = 12 bits > 8 bits available.
    BitBufferHandle handle(8, 4);
    bb.write(handle, value);
    Option<uint8_t> result = bb.read<uint8_t>(handle);
    // Expect that reading an out-of-range region returns none.
    TEST_ASSERT(result.isNone());
}

// Test an “invalid” range (here we choose a handle with zero size).
void test_BBInvalidRangeRW() {
    BitBuffer bb(8);
    uint8_t value = 0xAA;
    BitBufferHandle handle(0, 0); // Zero bits to read/write.
    bb.write(handle, value);
    Option<uint8_t> result = bb.read<uint8_t>(handle);
    // Expect that no valid data is read.
    TEST_ASSERT(result.isNone());
}

// Test read/write of various data types.
void test_BBTypeRW() {
    BitBuffer bb(32);  // 32-bit buffer.
    
    // Write and read an integer.
    int intValue = 123456;
    // Use a handle with size equal to sizeof(int)*8 bits starting at offset 0.
    BitBufferHandle intHandle(sizeof(int) * 8, 0);
    bb.write(intHandle, intValue);
    Option<int> intResult = bb.read<int>(intHandle);
    TEST_ASSERT(intResult.isSome());
    TEST_ASSERT_EQUAL_INT(intValue, intResult.value());

    // Now test a type that doesn’t fit in the allocated buffer.
    double doubleValue = 3.14159;
    // Use a 64-bit handle, but our buffer is only 32 bits.
    BitBufferHandle doubleHandle(64, 0);
    bb.write(doubleHandle, doubleValue);
    Option<double> doubleResult = bb.read<double>(doubleHandle);
    // Expect the read to fail.
    TEST_ASSERT(doubleResult.isNone());
}

// Test a series of random read/write operations in non-overlapping regions.
void test_BBRandomRW() {
    // Create a 64-bit buffer.
    BitBuffer bb(64);
    const int iterations = 10;
    for (int i = 0; i < iterations; ++i) {
        // Choose a random offset; we use multiples of 4 bits for simplicity.
        size_t offset = (std::rand() % 16) * 4;
        // Randomly choose a size from one of these: 4, 8, or 12 bits.
        size_t sizes[3] = {4, 8, 12};
        size_t sizeIndex = std::rand() % 3;
        size_t size = sizes[sizeIndex];
        if (offset + size > 64) {
            // Skip if the chosen region would exceed the buffer.
            continue;
        }
        // Generate a random value that fits in 'size' bits.
        uint32_t maxVal = (size < 32 ? (1u << size) - 1 : 0xFFFFFFFFu);
        uint32_t value = std::rand() % (maxVal + 1);
        BitBufferHandle handle(size, offset);
        bb.write(handle, value);
        Option<uint32_t> result = bb.read<uint32_t>(handle);
        TEST_ASSERT(result.isSome());
        // Compare only the lower 'size' bits.
        TEST_ASSERT_EQUAL_UINT32(value & maxVal, result.value() & maxVal);
    }
}

TEST_FUNC(test_BBInit);
TEST_FUNC(test_BBByteAlignedRW);
TEST_FUNC(test_BBWithinByteRW);
TEST_FUNC(test_BBCrossByteRW);
TEST_FUNC(test_BBOutOfRangeRW);
TEST_FUNC(test_BBInvalidRangeRW);
TEST_FUNC(test_BBTypeRW);
TEST_FUNC(test_BBRandomRW);
