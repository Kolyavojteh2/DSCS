#ifndef DSS_PROTOCOL_HANDLER_H
#define DSS_PROTOCOL_HANDLER_H

#include <cinttypes>
#include <vector>
#include <esp_mesh.h>

#ifndef DSS_PROTOCOL_TYPE_NUMBER
#define DSS_PROTOCOL_TYPE_NUMBER 12
#endif // DSS_PROTOCOL_TYPE_NUMBER

namespace DSSProtocolHandler
{
    static constexpr const char *moduleTag = "DSS_ProtocolHandler";

    void aliveNodeRequestHandler(const std::vector<uint8_t> &input);

    void bootstrapSend();
    void bootstrapRootHandler(const std::vector<uint8_t> &input);

    void chronoUpdateRequestSend();

    void chronoUpdateResponseHandler(const std::vector<uint8_t> &input);
    void chronoUpdateRequestHandler(const std::vector<uint8_t> &input);

    void getConfigRequestHandler(const std::vector<uint8_t> &input);
    void setConfigRequestHandler(const std::vector<uint8_t> &input);

    void getSensorsPacketRequestHandler(const std::vector<uint8_t> &input);
    void getSensorHeaderPacketRequestHandler(const std::vector<uint8_t> &input);
    void getSensorDataPacketRequestHandler(const std::vector<uint8_t> &input);

    void clearSensorDataRequestHandler(const std::vector<uint8_t> &input);
    void setSensorReadingModeRequestHandler(const std::vector<uint8_t> &input);
}

#endif // DSS_PROTOCOL_HANDLER_H