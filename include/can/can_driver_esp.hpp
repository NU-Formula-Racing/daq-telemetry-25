#ifndef __CAN_DRIVER_ESP_H__
#define __CAN_DRIVER_ESP_H__

#include <driver/gpio.h>
#include <driver/twai.h>
#include <freertos/FreeRTOS.h>

#include <can/can.hpp>

#define ESPCAN_DEFAULT_TX_PIN GPIO_NUM_21
#define ESPCAN_DEFAULT_RX_PIN GPIO_NUM_22

namespace can {

template <gpio_num_t txPin, gpio_num_t rxPin>
class ESPCANDriver : public CANDriver {
   public:
   
    DriverType getDriverType() override {
        return DT_INTERRUPT;
    }

    void install(CANBaudRate baudRate) override {
        _baudRate = baudRate;
        _genConfig = TWAI_GENERAL_CONFIG_DEFAULT(txPin, rxPin, TWAI_MODE_NORMAL);
        switch (_baudRate) {
            case CAN_100KBPS:
                _timingConfig = TWAI_TIMING_CONFIG_100KBITS();
                break;
            case CAN_125KBPS:
                _timingConfig = TWAI_TIMING_CONFIG_125KBITS();
                break;
            case CAN_250KBPS:
                _timingConfig = TWAI_TIMING_CONFIG_250KBITS();
                break;
            case CAN_500KBPS:
                _timingConfig = TWAI_TIMING_CONFIG_500KBITS();
                break;
            case CAN_1MBPS:
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

    void uninstall() override {
        twai_stop();
    }

   private:
    twai_general_config_t _genConfig;
    twai_timing_config_t _timingConfig;
    twai_filter_config_t _filterConfig;
    CANBaudRate _baudRate;
};

}  // namespace can

#endif  // __CAN_DRIVER_ESP_H__