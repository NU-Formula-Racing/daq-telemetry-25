#ifndef __TEST_DEBUG_H__
#define __TEST_DEBUG_H__

#include <debug.hpp>

#define TEST_DEBUG

#define TEST_SYSTEM_STR "TEST"
#define TEST_COLOR_STR COLOR_GREEN

#ifdef TEST_DEBUG

#define TEST_DEBUG_PRINT(fmt, ...) DEBUG_PRINT(TEST_SYSTEM_STR, TEST_COLOR_STR, fmt, ##__VA_ARGS__)
#define TEST_DEBUG_PRINTLN(fmt, ...) \
    DEBUG_PRINTLN(TEST_SYSTEM_STR, TEST_COLOR_STR, fmt, ##__VA_ARGS__)
#define TEST_DEBUG_PRINT_ERROR(fmt, ...) \
    DEBUG_PRINT_ERROR(TEST_SYSTEM_STR, TEST_COLOR_STR, fmt, ##__VA_ARGS__)
#define TEST_DEBUG_PRINT_ERRORLN(fmt, ...) \
    DEBUG_PRINT_ERRORLN(TEST_SYSTEM_STR, TEST_COLOR_STR, fmt, ##__VA_ARGS__)
#define TEST_DEBUG_PRINT_FATAL_ERROR(fmt, ...) \
    DEBUG_PRINT_FATAL_ERROR(TEST_SYSTEM_STR, TEST_COLOR_STR, fmt, ##__VA_ARGS__)
#define TEST_DEBUG_PRINT_FATAL_ERRORLN(fmt, ...) \
    DEBUG_PRINT_FATAL_ERRORLN(TEST_SYSTEM_STR, TEST_COLOR_STR, fmt, ##__VA_ARGS__)

#else  // TEST_DEBUG

#define TEST_DEBUG_PRINT(fmt, ...) (void)0
#define TEST_DEBUG_PRINTLN(fmt, ...) (void)0
#define TEST_DEBUG_PRINT_ERROR(fmt, ...) (void)0
#define TEST_DEBUG_PRINT_ERRORLN(fmt, ...) (void)0
#define TEST_DEBUG_PRINT_FATAL_ERROR(fmt, ...) (void)0
#define TEST_DEBUG_PRINT_FATAL_ERRORLN(fmt, ...) (void)0

#endif

#endif  // __TEST_DEBUG_H__