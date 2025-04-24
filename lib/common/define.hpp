#ifndef __DEFINE_H__
#define __DEFINE_H__

// debug flags
#define DEBUG  // global debug flag
#define CAN_DEBUG

// different pins for applciations
#if defined(APP_REMOTE)

#include <Arduino.h>  // for gpio pins

enum HWPin {
    // CAN Master Pins
    CM_DAQ_OPT = GPIO_NUM_36,
    CM_DRIVETRAIN_OPT = GPIO_NUM_39,
    CM_WHEEL_OPT = GPIO_NUM_36,
    CM_LED_CLK = GPIO_NUM_35,
    WLS_LORA_CS = GPIO_NUM_15,

    // Wireless Pins
    WLS_LORA_RST = GPIO_NUM_34,
    WLS_LORA_G0 = GPIO_NUM_32,
    WLS_DEBUG_CLK = GPIO_NUM_25,

    // Logger Pins
    LOGGER_DEBUG_CLK = GPIO_NUM_26,
    LOGGER_SD_CS = GPIO_NUM_2,
    LGR_RTC_SDA = GPIO_NUM_21,
    LGR_RTC_SCL = GPIO_NUM_22,

    // Webserver Pins
    WS_TASK = GPIO_NUM_16,

    // CAN Pins
    CAN_DRIVE_RX = GPIO_NUM_4,
    CAN_DRIVE_TX = GPIO_NUM_5,
    CAN_DATA_MCP_CS = GPIO_NUM_17,
    CAN_DATA_MCP_CLK = GPIO_NUM_18,
    CAN_DATA_MCP_MISO = GPIO_NUM_19,
    CAN_DATA_MCP_MOSI = GPIO_NUM_23,

    // General Pins
    SHIFT_REG_DATA = GPIO_NUM_27,

    // General Pins (don't use directly)
    SPI_CLK = GPIO_NUM_14,
    SPI_MISO = GPIO_NUM_12,
    SPI_MOSI = GPIO_NUM_13,
    ESP_RX = GPIO_NUM_3,
    ESP_TX = GPIO_NUM_1,
};

#elif defined(APP_BASE)

#include <Arduino.h>  // for gpio pins

// this is a native build, we can't have Arduino stuff
// this definition shouldn't really be used
enum HWPin {
    // CAN Master Pins
    CM_DAQ_OPT,
    CM_DRIVETRAIN_OPT,
    CM_WHEEL_OPT,
    WLS_LORA_CS,

    // Wireless Pins
    WLS_LORA_RST,
    WLS_DEBUG_CLK,

    // Logger Pins
    LOGGER_DEBUG_CLK,
    LOGGER_SD_CS,
    LGR_RTC_SDA,
    LGR_RTC_SCL,

    // Webserver Pins
    WS_TASK,

    // CAN Pins
    CAN_DRIVE_RX,
    CAN_DRIVE_TX,
    CAN_DATA_MCP_CS,
    CAN_DATA_MCP_CLK,
    CAN_DATA_MCP_MISO,
    CAN_DATA_MCP_MOSI,

    // General Pins
    SHIFT_REG_DATA,

    // General Pins (don't use directly)
    SPI_CLK,
    SPI_MISO,
    SPI_MOSI,
    ESP_RX,
    ESP_TX,
};

#else

// this is a native build, we can't have Arduino stuff
// this definition shouldn't really be used
enum HWPin {
    // CAN Master Pins
    CM_DAQ_OPT,
    CM_DRIVETRAIN_OPT,
    CM_WHEEL_OPT,
    WLS_LORA_CS,

    // Wireless Pins
    WLS_LORA_RST,
    WLS_DEBUG_CLK,

    // Logger Pins
    LOGGER_DEBUG_CLK,
    LOGGER_SD_CS,
    LGR_RTC_SDA,
    LGR_RTC_SCL,

    // Webserver Pins
    WS_TASK,

    // CAN Pins
    CAN_DRIVE_RX,
    CAN_DRIVE_TX,
    CAN_DATA_MCP_CS,
    CAN_DATA_MCP_CLK,
    CAN_DATA_MCP_MISO,
    CAN_DATA_MCP_MOSI,

    // General Pins
    SHIFT_REG_DATA,

    // General Pins (don't use directly)
    SPI_CLK,
    SPI_MISO,
    SPI_MOSI,
    ESP_RX,
    ESP_TX,
};

#endif
#endif
