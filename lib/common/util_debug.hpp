#ifndef __UTIL_DEBUG_H__
#define __UTIL_DEBUG_H__

#include <debug.hpp>

#define UTIL_SYSTEM_STR "UTIL"
#define UTIL_COLOR_STR COLOR_YELLOW

#ifdef UTIL_DEBUG

#define UTIL_DEBUG_PRINT(fmt, ...) DEBUG_PRINT(UTIL_SYSTEM_STR, UTIL_COLOR_STR, fmt, ##__VA_ARGS__)
#define UTIL_DEBUG_PRINTLN(fmt, ...) \
    DEBUG_PRINTLN(UTIL_SYSTEM_STR, UTIL_COLOR_STR, fmt, ##__VA_ARGS__)
#define UTIL_DEBUG_PRINT_ERROR(fmt, ...) \
    DEBUG_PRINT_ERROR(UTIL_SYSTEM_STR, UTIL_COLOR_STR, fmt, ##__VA_ARGS__)
#define UTIL_DEBUG_PRINT_ERRORLN(fmt, ...) \
    DEBUG_PRINT_ERRORLN(UTIL_SYSTEM_STR, UTIL_COLOR_STR, fmt, ##__VA_ARGS__)
#define UTIL_DEBUG_PRINT_FATAL_ERROR(fmt, ...) \
    DEBUG_PRINT_FATAL_ERROR(UTIL_SYSTEM_STR, UTIL_COLOR_STR, fmt, ##__VA_ARGS__)
#define UTIL_DEBUG_PRINT_FATAL_ERRORLN(fmt, ...) \
    DEBUG_PRINT_FATAL_ERRORLN(UTIL_SYSTEM_STR, UTIL_COLOR_STR, fmt, ##__VA_ARGS__)

#else  // UTIL_DEBUG

#define UTIL_DEBUG_PRINT(fmt, ...) (void)0
#define UTIL_DEBUG_PRINTLN(fmt, ...) (void)0
#define UTIL_DEBUG_PRINT_ERROR(fmt, ...) (void)0
#define UTIL_DEBUG_PRINT_ERRORLN(fmt, ...) (void)0
#define UTIL_DEBUG_PRINT_FATAL_ERROR(fmt, ...) (void)0
#define UTIL_DEBUG_PRINT_FATAL_ERRORLN(fmt, ...) (void)0

#endif

#endif  // __UTIL_DEBUG_H__