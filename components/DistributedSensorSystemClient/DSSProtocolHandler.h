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
}

#endif // DSS_PROTOCOL_HANDLER_H