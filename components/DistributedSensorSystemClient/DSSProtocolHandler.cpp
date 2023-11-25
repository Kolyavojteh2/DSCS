#include "DSSProtocolHandler.h"
#include "DSS_Protocol.h"
#include "MeshDataExchangeModule.h"

#include "packets/BootstrapPacket.h"

#include "packets/AliveNodeRequestPacket.h"
#include "packets/AliveNodeResponsePacket.h"

#include "packets/ChronoUpdateRequestPacket.h"
#include "packets/ChronoUpdateResponsePacket.h"

#include "packets/GetConfigPacketRequest.h"
#include "packets/GetConfigPacketResponse.h"
#include "packets/SetConfigPacket.h"

#include "packets/GetSensorsPacketRequest.h"
#include "packets/GetSensorsPacketResponse.h"
#include "packets/GetSensorDataPacketRequest.h"
#include "packets/GetSensorDataPacketResponse.h"
#include "packets/GetSensorHeaderPacketRequest.h"
#include "packets/GetSensorHeaderPacketResponse.h"
#include "packets/ClearSensorDataRequest.h"
#include "packets/SetSensorReadingModeRequest.h"

#include "ConfigurationManager.h"
#include "TimeManager.h"
#include "SensorManager.h"

#include <climits>

#include "WifiModule.h"
#include "MeshNetworkModule.h"
#include <esp_wifi.h>
#include <lwip/ip_addr.h>

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

        // Prepare DSS protocol packet
        // source MAC
        uint8_t *macAddress = WifiModule::getInstance().getApMAC();
        response.sourceMAC = std::vector<uint8_t>(macAddress, macAddress + 6);

        // destination MAC
        response.destinationMAC = request.sourceMAC;

        // AliveNodeResponsePacket
        AliveNodeResponsePacket_t *packet = dynamic_cast<AliveNodeResponsePacket_t *>(response.packet);

        // parent MAC
        packet->parentMAC = MeshNetworkModule::getInstance().getParentAddress();
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

    ESP_LOGI(moduleTag, "Sent bootstrap");

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

void DSSProtocolHandler::chronoUpdateRequestHandler(const std::vector<uint8_t> &input)
{
    // Check valid
    if (input.empty())
        return;

    if ((PacketType_t)input[DSS_PROTOCOL_TYPE_NUMBER] != PacketType_t::ChronoUpdateRequestPacket)
        return;

    auto prepareChronoUpdateResponsePacket = [](const std::vector<uint8_t> &input, DSS_Protocol_t &response)
    {
        // Get a request packet
        DSS_Protocol_t request(input);
        // ChronoUpdateRequestPacket_t *packetRequest = dynamic_cast<ChronoUpdateRequestPacket_t *>(request.packet);

        // ignore old time in a node
        // packetRequest->oldTime

        // Prepare DSS protocol packet
        uint8_t *macAddress = WifiModule::getInstance().getApMAC();
        response.sourceMAC = std::vector<uint8_t>(macAddress, macAddress + 6);
        response.destinationMAC = request.sourceMAC;

        // ChronoUpdateResponsePacket_t
        ChronoUpdateResponsePacket_t *packet = dynamic_cast<ChronoUpdateResponsePacket_t *>(response.packet);

        packet->newTime = TimeManager::getTime();
    };

    auto checkIsSendToNode = [](std::vector<uint8_t> &destinationMAC) -> bool
    {
        for (int i = 0; i < MAC_ADDRESS_LENGTH; ++i)
        {
            if (destinationMAC[i] != 0)
                return true;
        }

        return false;
    };

    DSS_Protocol_t response(PacketType_t::ChronoUpdateResponsePacket);
    prepareChronoUpdateResponsePacket(input, response);

    bool isSendingToNode = checkIsSendToNode(response.destinationMAC);

    std::vector<uint8_t> bin;
    response.toBin(bin);

    if (isSendingToNode)
        MeshDataExchangeModule::sendToNode(bin);
    else
        MeshDataExchangeModule::sendToIP(bin);
}

void DSSProtocolHandler::chronoUpdateResponseHandler(const std::vector<uint8_t> &input)
{
    // Check valid
    if (input.empty())
        return;

    if ((PacketType_t)input[DSS_PROTOCOL_TYPE_NUMBER] != PacketType_t::ChronoUpdateResponsePacket)
        return;

    // Get a request packet
    DSS_Protocol_t response(input);
    ChronoUpdateResponsePacket_t *packetRequest = dynamic_cast<ChronoUpdateResponsePacket_t *>(response.packet);
    TimeManager::setTime(packetRequest->newTime);
}

void DSSProtocolHandler::getConfigRequestHandler(const std::vector<uint8_t> &input)
{
    // Check valid
    if (input.empty())
        return;

    if ((PacketType_t)input[DSS_PROTOCOL_TYPE_NUMBER] != PacketType_t::GetConfigPacketRequest)
        return;

    auto prepareGetConfigResponsePacket = [](const std::vector<uint8_t> &input, DSS_Protocol_t &response)
    {
        // Get a request packet
        DSS_Protocol_t request(input);
        GetConfigPacketRequest_t *packetRequest = dynamic_cast<GetConfigPacketRequest_t *>(request.packet);

        // Prepare DSS protocol packet
        uint8_t *macAddress = WifiModule::getInstance().getApMAC();
        response.sourceMAC = std::vector<uint8_t>(macAddress, macAddress + 6);
        response.destinationMAC = request.sourceMAC;

        // GetConfigPacketResponse
        GetConfigPacketResponse_t *packet = dynamic_cast<GetConfigPacketResponse_t *>(response.packet);

        std::copy(packetRequest->ns.begin(), packetRequest->ns.end(), std::back_inserter(packet->ns));
        std::copy(packetRequest->configName.begin(), packetRequest->configName.end(), std::back_inserter(packet->configName));

        ConfigurationManager configManager(packet->ns);
        configManager.getConfiguration(packet->configName, packet->configValue);

        printf("Packet data: ns:%s, config:%s, value:%s\n", packet->ns.c_str(), packet->configName.c_str(), packet->configValue.c_str());
    };

    DSS_Protocol_t response(PacketType_t::GetConfigPacketResponse);
    prepareGetConfigResponsePacket(input, response);

    std::vector<uint8_t> bin;
    response.toBin(bin);

    MeshDataExchangeModule::sendToIP(bin);
}

void DSSProtocolHandler::setConfigRequestHandler(const std::vector<uint8_t> &input)
{
    // Check valid
    if (input.empty())
        return;

    if ((PacketType_t)input[DSS_PROTOCOL_TYPE_NUMBER] != PacketType_t::SetConfigPacket)
        return;

    // Get a request packet
    DSS_Protocol_t request(input);
    SetConfigPacket_t *packetRequest = dynamic_cast<SetConfigPacket_t *>(request.packet);

    ConfigurationManager configManager(packetRequest->ns);
    esp_err_t err = configManager.setConfiguration(packetRequest->configName, packetRequest->configValue);

    // TODO: add sending result of operation
}

void DSSProtocolHandler::chronoUpdateRequestSend()
{
    auto prepareChronoUpdateRequestPacket = [](DSS_Protocol_t &request)
    {
        // source MAC
        uint8_t *macAddress = WifiModule::getInstance().getApMAC();
        request.sourceMAC = std::vector<uint8_t>(macAddress, macAddress + 6);

        // destination MAC
        request.destinationMAC = std::vector<uint8_t>(MAC_ADDRESS_LENGTH, 0);
        ConfigurationManager configManager(SERVER_NAMESPACE_STR);

        // IP
        std::string ipStr;
        configManager.getConfiguration(SERVER_CONFIG_IP_STR, ipStr);
        ipaddr_aton(ipStr.c_str(), (ip_addr_t *)request.destinationMAC.data());
        // Port
        std::string portStr;
        configManager.getConfiguration(SERVER_CONFIG_PORT_STR, portStr);
        // TODO: clean code
        uint16_t port = atoi(portStr.c_str());
        request.destinationMAC[4] = ((uint8_t *)&port)[0];
        request.destinationMAC[5] = ((uint8_t *)&port)[1];

        // ChronoUpdateRequestPacket_t
        ChronoUpdateRequestPacket_t *packet = dynamic_cast<ChronoUpdateRequestPacket_t *>(request.packet);
        packet->oldTime = TimeManager::getTime();
    };

    ESP_LOGI(moduleTag, "Sent ChronoUpdateRequest");

    DSS_Protocol_t request(PacketType_t::ChronoUpdateRequestPacket);
    prepareChronoUpdateRequestPacket(request);

    std::vector<uint8_t> bin;
    request.toBin(bin);

    MeshDataExchangeModule::sendToIP(bin);
}

void DSSProtocolHandler::getSensorsPacketRequestHandler(const std::vector<uint8_t> &input)
{
    // Check valid
    if (input.empty())
        return;

    if ((PacketType_t)input[DSS_PROTOCOL_TYPE_NUMBER] != PacketType_t::GetSensorsPacketRequest)
        return;

    auto prepareGetSensorsPacketResponse = [](const std::vector<uint8_t> &input, DSS_Protocol_t &response)
    {
        // Get a request packet
        DSS_Protocol_t request(input);
        GetSensorsPacketRequest_t *packetRequest = dynamic_cast<GetSensorsPacketRequest_t *>(request.packet);

        // Prepare DSS protocol packet
        uint8_t *macAddress = WifiModule::getInstance().getApMAC();
        response.sourceMAC = std::vector<uint8_t>(macAddress, macAddress + 6);
        response.destinationMAC = request.sourceMAC;

        // GetSensorsPacketResponse
        GetSensorsPacketResponse_t *packet = dynamic_cast<GetSensorsPacketResponse_t *>(response.packet);

        std::list<std::string> list;
        SensorManager::getSensors(list);
        for (auto elem : list)
            packet->sensors.push_back(elem);
    };

    DSS_Protocol_t response(PacketType_t::GetSensorsPacketResponse);
    prepareGetSensorsPacketResponse(input, response);

    std::vector<uint8_t> bin;
    response.toBin(bin);

    MeshDataExchangeModule::sendToIP(bin);
}

void DSSProtocolHandler::getSensorHeaderPacketRequestHandler(const std::vector<uint8_t> &input)
{
    // Check valid
    if (input.empty())
        return;

    if ((PacketType_t)input[DSS_PROTOCOL_TYPE_NUMBER] != PacketType_t::GetSensorHeaderPacketRequest)
        return;

    auto prepareGetSensorHeaderPacketResponse = [](const std::vector<uint8_t> &input, DSS_Protocol_t &response)
    {
        // Get a request packet
        DSS_Protocol_t request(input);
        GetSensorHeaderPacketRequest_t *packetRequest = dynamic_cast<GetSensorHeaderPacketRequest_t *>(request.packet);

        // Prepare DSS protocol packet
        uint8_t *macAddress = WifiModule::getInstance().getApMAC();
        response.sourceMAC = std::vector<uint8_t>(macAddress, macAddress + 6);
        response.destinationMAC = request.sourceMAC;

        // GetSensorHeaderPacketResponse_t
        GetSensorHeaderPacketResponse_t *packet = dynamic_cast<GetSensorHeaderPacketResponse_t *>(response.packet);

        packet->sensorName = packetRequest->sensorName;

        std::map<std::string, uint8_t> header;
        SensorManager::getSensorHeader(packet->sensorName, header);
        for (auto &it : header)
            packet->sensorDataNames.push_back(it.first);
    };

    DSS_Protocol_t response(PacketType_t::GetSensorHeaderPacketResponse);
    prepareGetSensorHeaderPacketResponse(input, response);

    std::vector<uint8_t> bin;
    response.toBin(bin);

    MeshDataExchangeModule::sendToIP(bin);
}

void DSSProtocolHandler::getSensorDataPacketRequestHandler(const std::vector<uint8_t> &input)
{
    // Check valid
    if (input.empty())
        return;

    if ((PacketType_t)input[DSS_PROTOCOL_TYPE_NUMBER] != PacketType_t::GetSensorDataPacketRequest)
        return;

    auto prepareGetSensorDataPacketResponse = [](const std::vector<uint8_t> &input, DSS_Protocol_t &response)
    {
        // Get a request packet
        DSS_Protocol_t request(input);
        GetSensorDataPacketRequest_t *packetRequest = dynamic_cast<GetSensorDataPacketRequest_t *>(request.packet);

        // Prepare DSS protocol packet
        uint8_t *macAddress = WifiModule::getInstance().getApMAC();
        response.sourceMAC = std::vector<uint8_t>(macAddress, macAddress + 6);
        response.destinationMAC = request.sourceMAC;

        // GetSensorDataPacketResponse_t
        GetSensorDataPacketResponse_t *packet = dynamic_cast<GetSensorDataPacketResponse_t *>(response.packet);

        packet->sensorName = packetRequest->sensorName;
        packet->dataName = packetRequest->dataName;
        packet->sizeTime = sizeof(time_t);

        std::map<std::string, uint8_t> header;
        SensorManager::getSensorHeader(packet->sensorName, header);
        packet->sizeData = header[packet->dataName];

        SensorManager::getSensorData(packet->sensorName, packet->dataName, packet->dataTime, packet->dataValue);
    };

    DSS_Protocol_t response(PacketType_t::GetSensorDataPacketResponse);
    prepareGetSensorDataPacketResponse(input, response);

    std::vector<uint8_t> bin;
    response.toBin(bin);

    MeshDataExchangeModule::sendToIP(bin);
}

void DSSProtocolHandler::clearSensorDataRequestHandler(const std::vector<uint8_t> &input)
{
    // Check valid
    if (input.empty())
        return;

    if ((PacketType_t)input[DSS_PROTOCOL_TYPE_NUMBER] != PacketType_t::ClearSensorDataRequest)
        return;

    // Get a request packet
    DSS_Protocol_t request(input);
    ClearSensorDataRequest_t *packetRequest = dynamic_cast<ClearSensorDataRequest_t *>(request.packet);

    auto sensor = SensorManager::getSensor(packetRequest->sensorName);
    sensor->clearData(packetRequest->dataName, packetRequest->timeFrom, packetRequest->timeTo);
}

void DSSProtocolHandler::setSensorReadingModeRequestHandler(const std::vector<uint8_t> &input)
{
    // Check valid
    if (input.empty())
        return;

    if ((PacketType_t)input[DSS_PROTOCOL_TYPE_NUMBER] != PacketType_t::SetSensorReadingModeRequest)
        return;

    // Get a request packet
    DSS_Protocol_t request(input);
    SetSensorReadingModeRequest_t *packetRequest = dynamic_cast<SetSensorReadingModeRequest_t *>(request.packet);

    SensorManager::setReadingPeriod(packetRequest->period);
}
