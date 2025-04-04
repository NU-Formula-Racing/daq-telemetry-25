#ifndef __CAN_DEBUG_H__
#define __CAN_DEBUG_H__

#include <define.hpp>
#include <debug.hpp>

#define CAN_SYSTEM_STR "CAN"
#define CAN_COLOR_STR COLOR_CYAN

#ifdef CAN_DEBUG

#define CAN_DEBUG_PRINT(fmt, ...) DEBUG_PRINT(CAN_SYSTEM_STR, CAN_COLOR_STR, fmt, ##__VA_ARGS__)
#define CAN_DEBUG_PRINTLN(fmt, ...) DEBUG_PRINTLN(CAN_SYSTEM_STR, CAN_COLOR_STR, fmt, ##__VA_ARGS__)
#define CAN_DEBUG_PRINT_ERROR(fmt, ...) DEBUG_PRINT_ERROR(CAN_SYSTEM_STR, CAN_COLOR_STR, fmt, ##__VA_ARGS__)
#define CAN_DEBUG_PRINT_ERRORLN(fmt, ...) DEBUG_PRINT_ERRORLN(CAN_SYSTEM_STR, CAN_COLOR_STR, fmt, ##__VA_ARGS__)
#define CAN_DEBUG_PRINT_FATAL_ERROR(fmt, ...) DEBUG_PRINT_FATAL_ERROR(CAN_SYSTEM_STR, CAN_COLOR_STR, fmt, ##__VA_ARGS__)
#define CAN_DEBUG_PRINT_FATAL_ERRORLN(fmt, ...) DEBUG_PRINT_FATAL_ERRORLN(CAN_SYSTEM_STR, CAN_COLOR_STR, fmt, ##__VA_ARGS__)

#else  // CAN_DEBUG

#define CAN_DEBUG_PRINT(fmt, ...) (void)0
#define CAN_DEBUG_PRINTLN(fmt, ...) (void)0
#define CAN_DEBUG_PRINT_ERROR(fmt, ...) (void)0
#define CAN_DEBUG_PRINT_ERRORLN(fmt, ...) (void)0
#define CAN_DEBUG_PRINT_FATAL_ERROR(fmt, ...) (void)0
#define CAN_DEBUG_PRINT_FATAL_ERRORLN(fmt, ...) (void)0

#endif

#endif  // __CAN_DEBUG_H__