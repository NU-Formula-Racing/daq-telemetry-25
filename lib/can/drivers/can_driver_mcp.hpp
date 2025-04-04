#ifndef __CAN_DRIVER_MCP_H__
#define __CAN_DRIVER_MCP_H__

#include "define.hpp"

#ifndef __PLATFORM_NATIVE

#include <SPI.h>
#include <mcp2515.h>

#include "can/can.hpp"

namespace can {

template <HWPin csPin, uint8_t spiBusNum>
class MCPCanDriver : public CANDriver {
   public:
    DriverType getDriverType() {
        return DT_POLLING;
    }

    void install(CANBaudRate baudRate) {
        _spiBus = SPIClass(spiBusNum);
        _mcp = MCP2515(csPin, DEFAULT_SPI_CLOCK, &_spiBus);
        _mcp.reset();
        _mcp.setBitrate(_speedLUT[baudRate]);
        _mcp.setNormalMode();
    }

    void uninstall() {
        // no-op
    }

    void sendMessage(const RawCANMessage &message) {
        can_frame frame;
        frame.can_id = message.id;
        memccpy(frame.data, message.data, 8);
        _mcp.sendMessage(&frame);
    }

    bool recieveMessage(RawCANMessage *message) {
        can_frame frame;
        if (_mcp.readMessage(&frame) == MCP2515::ERROR_OK) {
            message->id = frame.can_id;
            message->length = frame.can_dlc;
            memccpy(message->data, frame.data, 8);
            return true;
        }
        return false;
    }

   private:
    MCP2515 _mcp;
    SPIClass _spiBus;

    static CAN_SPEED _speedLUT[] = [
        CAN_100KBPS,
        CAN_125KBPS,
        CAN_250KBPS,
        CAN_500KBPS,
        CAN_100KBPS
    ];
};

}  // namespace can

#endif // !__PLATFORM_NATIVE

#endif  // __CAN_DRIVER_MCP_H__