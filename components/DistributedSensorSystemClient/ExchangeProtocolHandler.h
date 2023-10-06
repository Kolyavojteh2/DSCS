#ifndef EXCHANGE_PROTOCOL_HANDLER_H
#define EXCHANGE_PROTOCOL_HANDLER_H

#include <cinttypes>
#include <vector>
#include <esp_mesh.h>

namespace ExchangeProtocolHandler
{
    void aliveNodeRequestHandler(const mesh_addr_t &source, const std::vector<uint8_t> &data);
}

#endif // EXCHANGE_PROTOCOL_HANDLER_H