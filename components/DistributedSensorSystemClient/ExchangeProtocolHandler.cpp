#include "ExchangeProtocolHandler.h"
#include "ExchangeProtocol.h"
#include "MeshDataExchangeModule.h"
#include "packets/AliveNodePackets.h"
#include "WifiModule.h"

void ExchangeProtocolHandler::aliveNodeRequestHandler(const mesh_addr_t &source, const std::vector<uint8_t> &data)
{
    // Check valid
    if (data.empty())
        return;

    if ((PacketType_t)data[0] != PacketType_t::AliveNodeRequest)
        return;

    auto prepareAliveNodeResponse = [](const std::vector<uint8_t> &data, ExchangeProtocol_t &response)
    {
        ExchangeProtocol_t request(data);
        AliveNodeRequest_t *packetRequest = dynamic_cast<AliveNodeRequest_t *>(request.packet);

        uint8_t *macAddress = WifiModule::getInstance().getApMAC();
        response.deviceMAC = std::vector<uint8_t>(macAddress, macAddress + 6);

        AliveNodeRequest_t *packetResponse = dynamic_cast<AliveNodeRequest_t *>(response.packet);
        packetResponse->operationID = packetRequest->operationID;
    };

    ExchangeProtocol_t response(PacketType_t::AliveNodeResponse);
    prepareAliveNodeResponse(data, response);

    MeshExchangePacket_t meshPacket;
    response.toBin(meshPacket.data);

    MeshDataExchangeModule::sendToIP(&source, meshPacket);
}