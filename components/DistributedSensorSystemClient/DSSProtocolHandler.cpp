#include "DSSProtocolHandler.h"
#include "DSS_Protocol.h"
#include "MeshDataExchangeModule.h"
#include "packets/AliveNodeRequestPacket.h"
#include "packets/AliveNodeResponsePacket.h"
#include "packets/BootstrapPacket.h"

#include "WifiModule.h"
#include "MeshNetworkModule.h"
#include <esp_wifi.h>

void DSSProtocolHandler::aliveNodeRequestHandler(const std::vector<uint8_t> &input)
{
    // Check valid
    if (input.empty())
        return;

    if ((PacketType_t)input[DSS_PROTOCOL_TYPE_NUMBER] != PacketType_t::AliveNodeRequestPacket)
        return;

    auto prepareAliveNodeResponsePacket = [](const std::vector<uint8_t> &input, DSS_Protocol_t &response)
    {
        // Get a request packet
        DSS_Protocol_t request(input);
        AliveNodeRequestPacket_t *packetRequest = dynamic_cast<AliveNodeRequestPacket_t *>(request.packet);

        // Prepare DSS protocol packet
        // source MAC
        uint8_t *macAddress = WifiModule::getInstance().getApMAC();
        response.sourceMAC = std::vector<uint8_t>(macAddress, macAddress + 6);

        // destination MAC
        response.destinationMAC = request.sourceMAC;

        // Packet response is empty
    };

    DSS_Protocol_t response(PacketType_t::AliveNodeResponsePacket);
    prepareAliveNodeResponsePacket(input, response);

    std::vector<uint8_t> bin;
    response.toBin(bin);

    MeshDataExchangeModule::sendToIP(bin);
}

void DSSProtocolHandler::bootstrapSend()
{
    auto prepareBootstrapPacket = [](DSS_Protocol_t &bootstrap)
    {
        // source MAC
        uint8_t *macAddress = WifiModule::getInstance().getApMAC();
        bootstrap.sourceMAC = std::vector<uint8_t>(macAddress, macAddress + 6);

        // TODO: look at this place with 00:00:00:00:00:00 destination MAC as root
        bootstrap.destinationMAC = std::vector<uint8_t>(MAC_ADDRESS_LENGTH, 0);

        // BootstrapPacket
        BootstrapPacket_t *packet = dynamic_cast<BootstrapPacket_t *>(bootstrap.packet);

        // channel
        wifi_second_chan_t secondChan;
        esp_wifi_get_channel(&(packet->channel), &secondChan);

        // rootMAC
        packet->rootMAC = MeshNetworkModule::getInstance().getRootAddress();

        // network ID
        mesh_addr_t networkID;
        esp_mesh_get_id(&networkID);
        packet->networkID = std::vector<uint8_t>(networkID.addr, networkID.addr + NETWORK_ID_LENGTH);
    };

    DSS_Protocol_t bootstrap(PacketType_t::BootstrapPacket);
    prepareBootstrapPacket(bootstrap);

    std::vector<uint8_t> bin;
    bootstrap.toBin(bin);

    MeshDataExchangeModule::sendToRoot(bin);
}

void DSSProtocolHandler::bootstrapRootHandler(const std::vector<uint8_t> &input)
{
    // Check valid
    if (input.empty())
        return;

    if (!esp_mesh_is_root())
        return;

    if ((PacketType_t)input[DSS_PROTOCOL_TYPE_NUMBER] != PacketType_t::BootstrapPacket)
        return;

    // Just retransmit it
    MeshDataExchangeModule::sendToIP(input);
}