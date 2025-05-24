#ifndef __CAN_DRIVER_MCP_H__
#define __CAN_DRIVER_MCP_H__

#include <define.hpp>

#ifndef __PLATFORM_NATIVE

#include <SPI.h>
#include <can.h>
#include <mcp2515.h>

namespace can {

static CAN_SPEED __speedLUT[] = {CAN_100KBPS, CAN_125KBPS, CAN_250KBPS, CAN_500KBPS, CAN_100KBPS};

template <HWPin csPin, uint8_t spiBusNum>
class MCPCanDriver : public CANDriver {
   public:
    MCPCanDriver() : _spiBus(spiBusNum), _mcp(csPin, 10000000U, &_spiBus) {}

    DriverType getDriverType() { return DT_POLLING; }

    void install(CANBaudRate baudRate) {
        _mcp.reset();
        _mcp.setBitrate(__speedLUT[baudRate]);
        _mcp.setNormalMode();
    }

    void uninstall() {
        // no-op
    }

    void sendMessage(const RawCANMessage& message) {
        can_frame frame;
        frame.can_id = message.id;
        memcpy(frame.data, message.data, 8);
        _mcp.sendMessage(&frame);
    }

    bool receiveMessage(RawCANMessage* message) {
        can_frame frame;
        if (_mcp.readMessage(&frame) == MCP2515::ERROR_OK) {
            message->id = frame.can_id;
            message->length = frame.can_dlc;
            memcpy(message->data, frame.data, 8);
            return true;
        }
        return false;
    }

   private:
    SPIClass _spiBus;
    MCP2515 _mcp;
};

}  // namespace can

#endif  // !__PLATFORM_NATIVE

#endif  // __CAN_DRIVER_MCP_H__