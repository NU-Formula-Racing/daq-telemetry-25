#ifndef __TELEM_DEBUG_H__
#define __TELEM_DEBUG_H__

#include <debug.hpp>

#define TELEM_SYSTEM_STR "TELEM"
#define TELEM_COLOR_STR COLOR_BLUE

#ifdef TELEM_DEBUG

#define TELEM_DEBUG_PRINT(fmt, ...) \
    DEBUG_PRINT(TELEM_SYSTEM_STR, TELEM_COLOR_STR, fmt, ##__VA_ARGS__)
#define TELEM_DEBUG_PRINTLN(fmt, ...) \
    DEBUG_PRINTLN(TELEM_SYSTEM_STR, TELEM_COLOR_STR, fmt, ##__VA_ARGS__)
#define TELEM_DEBUG_PRINT_ERROR(fmt, ...) \
    DEBUG_PRINT_ERROR(TELEM_SYSTEM_STR, TELEM_COLOR_STR, fmt, ##__VA_ARGS__)
#define TELEM_DEBUG_PRINT_ERRORLN(fmt, ...) \
    DEBUG_PRINT_ERRORLN(TELEM_SYSTEM_STR, TELEM_COLOR_STR, fmt, ##__VA_ARGS__)
#define TELEM_DEBUG_PRINT_FATAL_ERROR(fmt, ...) \
    DEBUG_PRINT_FATAL_ERROR(TELEM_SYSTEM_STR, TELEM_COLOR_STR, fmt, ##__VA_ARGS__)
#define TELEM_DEBUG_PRINT_FATAL_ERRORLN(fmt, ...) \
    DEBUG_PRINT_FATAL_ERRORLN(TELEM_SYSTEM_STR, TELEM_COLOR_STR, fmt, ##__VA_ARGS__)

#else  // TELEM_DEBUG

#define TELEM_DEBUG_PRINT(fmt, ...) (void)0
#define TELEM_DEBUG_PRINTLN(fmt, ...) (void)0
#define TELEM_DEBUG_PRINT_ERROR(fmt, ...) (void)0
#define TELEM_DEBUG_PRINT_ERRORLN(fmt, ...) (void)0
#define TELEM_DEBUG_PRINT_FATAL_ERROR(fmt, ...) (void)0
#define TELEM_DEBUG_PRINT_FATAL_ERRORLN(fmt, ...) (void)0

#endif

#endif  // __TELEM_DEBUG_H__