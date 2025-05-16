#ifndef __CAN_DRIVER_ESP_H__
#define __CAN_DRIVER_ESP_H__

#include <define.hpp>

#ifdef __PLATFORM_ESP

#include <driver/gpio.h>
#include <driver/twai.h>
#include <freertos/FreeRTOS.h>

#include <can.hpp>

#define ESPCAN_DEFAULT_TX_PIN GPIO_NUM_21
#define ESPCAN_DEFAULT_RX_PIN GPIO_NUM_22

namespace can {

template <gpio_num_t txPin, gpio_num_t rxPin>
class ESPCANDriver : public CANDriver {
   public:
    DriverType getDriverType() { return DT_POLLING; }

    void install(CANBaudRate baudRate) {
        _baudRate = baudRate;
        _genConfig = TWAI_GENERAL_CONFIG_DEFAULT(txPin, rxPin, TWAI_MODE_NORMAL);
        switch (_baudRate) {
            case CBR_100KBPS:
                _timingConfig = TWAI_TIMING_CONFIG_100KBITS();
                break;
            case CBR_125KBPS:
                _timingConfig = TWAI_TIMING_CONFIG_125KBITS();
                break;
            case CBR_250KBPS:
                _timingConfig = TWAI_TIMING_CONFIG_250KBITS();
                break;
            case CBR_500KBPS:
                _timingConfig = TWAI_TIMING_CONFIG_500KBITS();
                break;
            case CBR_1MBPS:
                _timingConfig = TWAI_TIMING_CONFIG_1MBITS();
                break;
            default:
                _timingConfig = TWAI_TIMING_CONFIG_100KBITS();
                break;
        }

        _filterConfig = TWAI_FILTER_CONFIG_ACCEPT_ALL();

        // Install the TWAI driver.
        twai_driver_install(&_genConfig, &_timingConfig, &_filterConfig);

        // Start the TWAI driver.
        twai_start();
    }

    void uninstall() {
        twai_stop();
        twai_driver_uninstall();
    }

    void sendMessage(const RawCANMessage& message) {
        twai_message_t twaiMessage;
        twaiMessage.identifier = message.id;
        twaiMessage.data_length_code = message.length;
        memcpy(twaiMessage.data, message.data, sizeof(message.data));
        twai_transmit(&twaiMessage, portMAX_DELAY);
    }

    bool receiveMessage(RawCANMessage* message) {
        twai_message_t twaiMessage;
        if (twai_receive(&twaiMessage, portMAX_DELAY) == ESP_OK) {
            message->id = twaiMessage.identifier;
            message->length = twaiMessage.data_length_code;
            memcpy(message->data, twaiMessage.data, sizeof(message->data));
            return true;
        }
        return false;
    }

    void clearTransmitQueue() { twai_clear_transmit_queue(); }

    void clearReceiveQueue() { twai_clear_receive_queue(); }

   private:
    twai_general_config_t _genConfig;
    twai_timing_config_t _timingConfig;
    twai_filter_config_t _filterConfig;
    CANBaudRate _baudRate;
};

}  // namespace can

#endif  // __PLATFORM_ESP

#endif  // __CAN_DRIVER_ESP_H__