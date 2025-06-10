#ifndef __CAN_DRIVER_ESP_H__
#define __CAN_DRIVER_ESP_H__

#include <define.hpp>

#ifdef __PLATFORM_ESP

#include <driver/gpio.h>
#include <driver/twai.h>
#include <freertos/FreeRTOS.h>

#include <array>
#include <can.hpp>

#define ESPCAN_DEFAULT_TX_PIN GPIO_NUM_5
#define ESPCAN_DEFAULT_RX_PIN GPIO_NUM_4

namespace can {

template <gpio_num_t txPin, gpio_num_t rxPin>
class ESPCANDriver : public CANDriver {
   public:
    DriverType getDriverType() override { return DT_POLLING; }

    void install(CANBaudRate baudRate) override {
        _baudRate = baudRate;
        _genConfig = TWAI_GENERAL_CONFIG_DEFAULT(txPin, rxPin, TWAI_MODE_NORMAL);
        _genConfig.rx_queue_len = RX_BUFFER_SIZE;
        _timingConfig = selectTiming(baudRate);
        _filterConfig = TWAI_FILTER_CONFIG_ACCEPT_ALL();

        twai_driver_install(&_genConfig, &_timingConfig, &_filterConfig);
        twai_start();
        CAN_DEBUG_PRINTLN("Installed ESP32 CAN driver!");
    }

    void uninstall() override {
        twai_stop();
        twai_driver_uninstall();
    }

    void sendMessage(const RawCANMessage& msg) override {
        twai_message_t tx;
        tx.identifier = msg.id;
        tx.data_length_code = msg.length;
        memcpy(tx.data, msg.data, msg.length);
        twai_transmit(&tx, portMAX_DELAY);
    }

    bool receiveMessage(RawCANMessage* out) override {
        // if there's anything buffered, hand it out
        tick();

        if (_rxCount > 0) {
            *out = _rxBuf[_rxTail];
            _rxTail = (_rxTail + 1) % RX_BUFFER_SIZE;
            --_rxCount;
            return true;
        }
        return false;
    }

    void tick() {
        // fetch status
        twai_status_info_t status;
        twai_get_status_info(&status);

        // bus-off recovery / auto-restart
        if (status.state == TWAI_STATE_BUS_OFF) {
            twai_initiate_recovery();
            CAN_DEBUG_PRINTLN("TWAI bus off—initiating recovery");
        }
        if (status.state == TWAI_STATE_STOPPED) {
            twai_start();
            CAN_DEBUG_PRINTLN("TWAI was stopped—restarting");
        }

        // drain all pending HW messages
        while (status.msgs_to_rx > 0) {
            if (status.rx_missed_count > 0) {
                CAN_DEBUG_PRINT_ERRORLN("Missed %u CAN msgs due to full HW queue",
                                        status.rx_missed_count);
            }

            twai_message_t hwMsg;
            if (twai_receive(&hwMsg, (TickType_t)100) == ESP_OK) {
                RawCANMessage raw;
                raw.id = hwMsg.identifier;
                raw.length = hwMsg.data_length_code;
                memcpy(raw.data, hwMsg.data, hwMsg.data_length_code);

                // enqueue into our circular buffer
                if (_rxCount < RX_BUFFER_SIZE) {
                    _rxBuf[_rxHead] = raw;
                    _rxHead = (_rxHead + 1) % RX_BUFFER_SIZE;
                    ++_rxCount;
                } else {
                    CAN_DEBUG_PRINT_ERRORLN("RX buffer full, dropping incoming message");
                }

                // CAN_DEBUG_PRINTLN("Tick: received id=%u len=%u", raw.id, raw.length);
            } else {
                CAN_DEBUG_PRINT_ERRORLN("Failed to read message from TWAI queue");
            }

            // refresh status for next iteration
            twai_get_status_info(&status);
        }
    }

    void clearTransmitQueue() override { twai_clear_transmit_queue(); }
    void clearReceiveQueue() override {
        twai_clear_receive_queue();
        _rxHead = _rxTail = _rxCount = 0;
    }

   private:
    static constexpr size_t RX_BUFFER_SIZE = 64;
    std::array<RawCANMessage, RX_BUFFER_SIZE> _rxBuf{};
    size_t _rxHead = 0;
    size_t _rxTail = 0;
    size_t _rxCount = 0;

    twai_general_config_t _genConfig;
    twai_timing_config_t _timingConfig;
    twai_filter_config_t _filterConfig;
    CANBaudRate _baudRate;

    static twai_timing_config_t selectTiming(CANBaudRate br) {
        switch (br) {
            case CBR_100KBPS:
                return TWAI_TIMING_CONFIG_100KBITS();
            case CBR_125KBPS:
                return TWAI_TIMING_CONFIG_125KBITS();
            case CBR_250KBPS:
                return TWAI_TIMING_CONFIG_250KBITS();
            case CBR_500KBPS:
                return TWAI_TIMING_CONFIG_500KBITS();
            case CBR_1MBPS:
                return TWAI_TIMING_CONFIG_1MBITS();
            default:
                return TWAI_TIMING_CONFIG_100KBITS();
        }
    }
};

}  // namespace can

#endif  // __PLATFORM_ESP
#endif  // __CAN_DRIVER_ESP_H__
