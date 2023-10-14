#include "MeshDataExchangeModule.h"
#include "MeshNetworkModule.h"
#include "WifiModule.h"
#include "DSS_Protocol.h"

#include <esp_mesh.h>
#include <esp_log.h>

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"

#include <vector>

#ifndef MAC_ADDRESS_LENGTH
#define MAC_ADDRESS_LENGTH (6)
#endif // MAC_ADDRESS_LENGTH

PacketHandleMechanism MeshDataExchangeModule::m_mechanismFromExternalIP;
PacketHandleMechanism MeshDataExchangeModule::m_mechanismToExternalIP;
PacketHandleMechanism MeshDataExchangeModule::m_mechanismToNode;

PacketHandleMechanism MeshDataExchangeModule::m_mechanismPacketHandlers;

MeshDataExchangeModule &MeshDataExchangeModule::getInstance()
{
    static MeshDataExchangeModule instance;

    return instance;
}

MeshDataExchangeModule::MeshDataExchangeModule()
{
    const int maxSemaphoreCount = 2;
    m_receivingSemaphore = xSemaphoreCreateCounting(maxSemaphoreCount, 0);

    const int stackSize = 4096;
    const int priorityReceiving = 1;
    const int coreNumber = 1;
    xTaskCreatePinnedToCore(receiveMeshTask, "receiveMeshTask", stackSize, NULL, priorityReceiving, NULL, coreNumber);
    xTaskCreatePinnedToCore(receiveIPTask, "receiveIPTask", stackSize, NULL, priorityReceiving, NULL, coreNumber);
}

bool MeshDataExchangeModule::checkData(const std::vector<uint8_t> &data)
{
    // If the data size is equal 0, return
    if (data.size() == 0)
    {
        ESP_LOGE(moduleTag, "The data size is equal 0");
        return ESP_ERR_MESH_ARGUMENT;
    }

    // If data bigger than the MPS, return
    if (data.size() > MESH_MPS)
    {
        ESP_LOGE(moduleTag, "The data size is bigger than MPS(1472)");
        return ESP_ERR_MESH_EXCEED_MTU;
    }

    return ESP_OK;
}

void MeshDataExchangeModule::prepareSendPacket(mesh_data_t &sendPacket, const std::vector<uint8_t> &bin)
{
    // TODO: look if the memory of out, create allocate memory via new or malloc
    sendPacket.data = (uint8_t *)bin.data();
    sendPacket.size = bin.size();
    // TODO: add possibility for JSON and HTTP
    sendPacket.proto = MESH_PROTO_BIN;
    sendPacket.tos = MESH_TOS_P2P;
}

esp_err_t MeshDataExchangeModule::sendToRoot(const std::vector<uint8_t> &bin)
{
    esp_err_t ret = checkData(bin);
    if (ret)
        return ret;

    mesh_data_t dataDes;
    prepareSendPacket(dataDes, bin);

    int sendFlag = MeshPacketFlag_t::ToNode;

    // To root specific code
    ret = esp_mesh_send(NULL, &dataDes, sendFlag, NULL, 0);
    if (ret)
    {
        ESP_LOGE(moduleTag, "The message doesn't sent. Error code: %d", ret);
        // TODO: add hander with different errors and may be retry mechanism
    }

    return ret;
}

esp_err_t MeshDataExchangeModule::sendToNode(const std::vector<uint8_t> &bin)
{
    esp_err_t ret = checkData(bin);
    if (ret)
        return ret;

    mesh_data_t dataDes;
    prepareSendPacket(dataDes, bin);

    int sendFlag = MeshPacketFlag_t::ToNode;

    mesh_addr_t destinationAddress;
    DSS_Protocol_t header = DSS_Protocol_t::makeHeaderDataOnly(bin);
    memcpy(destinationAddress.addr, header.destinationMAC.data(), MAC_ADDRESS_LENGTH);

    ret = esp_mesh_send(&destinationAddress, &dataDes, sendFlag, NULL, 0);
    if (ret)
    {
        ESP_LOGE(moduleTag, "The message doesn't sent. Error code: %d", ret);
        // TODO: add hander with different errors and may be retry mechanism
    }

    return ret;
}

esp_err_t MeshDataExchangeModule::sendToIP(const std::vector<uint8_t> &bin)
{
    if (esp_mesh_is_root())
        return sendToIPAsRoot(bin);

    return sendToIPAsNode(bin);
}

esp_err_t MeshDataExchangeModule::sendToIPAsNode(const std::vector<uint8_t> &bin)
{
    esp_err_t ret = checkData(bin);
    if (ret)
        return ret;

    mesh_data_t dataDes;
    prepareSendPacket(dataDes, bin);

    int sendFlag = MeshPacketFlag_t::ToIP;

    // Here IP instead MAC
    mesh_addr_t destinationAddress;
    DSS_Protocol_t header = DSS_Protocol_t::makeHeaderDataOnly(bin);
    memcpy(destinationAddress.addr, header.destinationMAC.data(), MAC_ADDRESS_LENGTH);

    ret = esp_mesh_send(&destinationAddress, &dataDes, sendFlag, NULL, 0);
    if (ret)
    {
        ESP_LOGE(moduleTag, "The message doesn't sent. Error code: %d", ret);
        // TODO: add hander with different errors and may be retry mechanism
    }

    return ret;
}

esp_err_t MeshDataExchangeModule::sendToIPAsRoot(const std::vector<uint8_t> &bin)
{
    int socket_fd = MeshNetworkModule::getInstance().getSocket();

    int sentBytes = write(socket_fd, bin.data(), bin.size());
    if (sentBytes != bin.size())
    {
        // TODO: add error handling
        ESP_LOGE(moduleTag, "TCP send(write) error. Sent bytes: %d. Packet size: %d", sentBytes, bin.size());
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t MeshDataExchangeModule::sendFromIP(const std::vector<uint8_t> &bin)
{
    esp_err_t ret = checkData(bin);
    if (ret)
        return ret;

    mesh_data_t dataDes;
    prepareSendPacket(dataDes, bin);

    int sendFlag = MeshPacketFlag_t::FromIP;

    mesh_addr_t destinationAddress;
    DSS_Protocol_t header = DSS_Protocol_t::makeHeaderDataOnly(bin);
    memcpy(destinationAddress.addr, header.destinationMAC.data(), MAC_ADDRESS_LENGTH);

    ret = esp_mesh_send(&destinationAddress, &dataDes, sendFlag, NULL, 0);
    if (ret)
    {
        ESP_LOGE(moduleTag, "The message doesn't sent. Error code: %d", ret);
        // TODO: add hander with different errors and may be retry mechanism
    }

    return ret;
}

void MeshDataExchangeModule::startReceiving(void)
{
    m_receivingState = true;
    // Need give semaphore 2 times
    xSemaphoreGive(m_receivingSemaphore);
    xSemaphoreGive(m_receivingSemaphore);
}

void MeshDataExchangeModule::stopReceiving(void)
{
    m_receivingState = false;
}

void MeshDataExchangeModule::receiveIPTask(void * /*unused*/)
{
    uint8_t recvBuffer[MESH_MTU];
    ssize_t gotBytes = 0;

    while (1)
    {
        int socket_fd = MeshNetworkModule::getInstance().getSocket();

        // TODO: add sync with semaphores. (remove comment after testing)
        // Check the receiving state
        if (MeshDataExchangeModule::getInstance().m_receivingState == false || socket_fd <= 0)
            xSemaphoreTake(MeshDataExchangeModule::getInstance().m_receivingSemaphore, portMAX_DELAY);

        gotBytes = read(socket_fd, recvBuffer, MESH_MTU);
        if (gotBytes < 0)
        {
            // TODO: add handling errno errors
            continue;
        }

        // TODO: check if it is needing
        ESP_LOGI(moduleTag, "Received data from MESH: %s", recvBuffer);

        std::vector<uint8_t> bin(recvBuffer, recvBuffer + gotBytes);
        int recvFlag = MeshPacketFlag_t::FromIP;
        analyzeAndProcessData(bin, recvFlag);
    }
}

void MeshDataExchangeModule::receiveMeshTask(void * /*unused*/)
{
    esp_err_t recvRet;            // Used to store the result of the recv()
    uint8_t recvBuffer[MESH_MTU]; // Receive Buffer (MESH_MTU = 1500 bytes)
    mesh_addr_t source = {0};     // Received packet's source address
    int recvFlag = 0;             // Received packet's flag
    mesh_opt_t recvOpt;           // Received packet's additional option
    uint8_t optVal[1];            // Received packet's additional option's value

    recvOpt.val = optVal;

    mesh_data_t dataDes = {.data = recvBuffer, .size = MESH_MTU, .proto = MESH_PROTO_BIN, .tos = MESH_TOS_P2P}; // Received packet's data descriptor

    while (1)
    {
        // Check the receiving state
        if (MeshDataExchangeModule::getInstance().m_receivingState == false)
            xSemaphoreTake(MeshDataExchangeModule::getInstance().m_receivingSemaphore, portMAX_DELAY);

        dataDes.size = MESH_MTU;
        // TODO: check if needing it
        recvOpt.type = 0;

        // Receive the packet
        recvRet = esp_mesh_recv(&source, &dataDes, portMAX_DELAY, &recvFlag, &recvOpt, 1);
        if (recvRet != ESP_OK)
        {
            // TODO: add error handling
            // mesh_receive_error_handler(recvRet, &source, &dataDes, portMAX_DELAY, recvFlag, &recvOpt);
            continue;
        }

        // TODO: check if it is needing
        ESP_LOGI(moduleTag, "Received data from MESH: %s", dataDes.data);

        std::vector<uint8_t> bin(dataDes.data, dataDes.data + dataDes.size);
        analyzeAndProcessData(bin, recvFlag);

        /*
        // Receive mechanism handlers
        if (isToExternalIP(recvFlag))
        {
            ESP_LOGI(moduleTag, "Destination: toDS");
            // from node to external IP
            m_mechanismToExternalIP.run(source, dataDes);
        }

        if (isFromExternalIPToNode(recvFlag))
        {
            ESP_LOGI(moduleTag, "Destination: fromDS");
            // from external IP
            m_mechanismFromExternalIP.run(source, dataDes);
        }

        if (isFromNodeToNode(recvFlag))
        {
            ESP_LOGI(moduleTag, "Destination: P2P");
            // from node
            m_mechanismToNode.run(source, dataDes);
        }
        */
    }
}

void MeshDataExchangeModule::analyzeAndProcessData(const std::vector<uint8_t> bin, const int flag)
{
    DSS_Protocol_t header = DSS_Protocol_t::makeHeaderDataOnly(bin);

    if (isNeedToHandle(header, flag))
    {
        ESP_LOGI(moduleTag, "Handle this packet.");
        handleReceivedData(bin, flag);
    }

    if (isNeedToRetransmit(header, flag))
    {
        ESP_LOGI(moduleTag, "Retransmit this packet.");
        retransmitReceivedData(bin, flag);
    }
}

void MeshDataExchangeModule::handleReceivedData(const std::vector<uint8_t> bin, const int flag)
{
    // TODO: remove argument flag or use it
    (void)flag;

    m_mechanismPacketHandlers.run(bin);
}

void MeshDataExchangeModule::retransmitReceivedData(const std::vector<uint8_t> bin, const int flag)
{
    switch (flag)
    {
    case MeshPacketFlag_t::ToIP:
        sendToIP(bin);
        break;

    case MeshPacketFlag_t::ToRoot:
        sendToRoot(bin);
        break;

    case MeshPacketFlag_t::ToNode:
        sendToNode(bin);
        break;

    case MeshPacketFlag_t::FromIP:
        sendFromIP(bin);
        break;

    default:
        ESP_LOGE(moduleTag, "Unknown the MeshPacketFlag_t flag. Code: %d", flag);
        break;
    }
}

bool MeshDataExchangeModule::isDestinationCurrentDevice(const DSS_Protocol_t &data)
{
    std::vector<std::vector<uint8_t>> currentDeviceMACs;

    uint8_t *apMAC = WifiModule::getInstance().getApMAC();
    uint8_t *staMAC = WifiModule::getInstance().getStaMAC();

    currentDeviceMACs.push_back(std::vector<uint8_t>(apMAC, apMAC + MAC_ADDRESS_LENGTH));
    currentDeviceMACs.push_back(std::vector<uint8_t>(staMAC, staMAC + MAC_ADDRESS_LENGTH));

    for (int i = 0; i < currentDeviceMACs.size(); ++i)
    {
        if (data.destinationMAC == currentDeviceMACs[i])
            return true;
    }

    return false;
}

bool MeshDataExchangeModule::isBroadcast(const DSS_Protocol_t &header)
{
    // TODO: add check broadcast
    return false;
}

bool MeshDataExchangeModule::isNeedToHandle(const DSS_Protocol_t &header, const int flag)
{
    if (isBroadcast(header))
        return true;

    if (flag == MeshPacketFlag_t::ToRoot && esp_mesh_is_root())
        return true;

    if ((flag == MeshPacketFlag_t::ToNode || flag == MeshPacketFlag_t::FromIP) &&
        isDestinationCurrentDevice(header))
        return true;

    return false;
}

bool MeshDataExchangeModule::isNeedToRetransmit(const DSS_Protocol_t &header, const int flag)
{
    if (isBroadcast(header))
        return true;

    if (flag == MeshPacketFlag_t::ToRoot && !esp_mesh_is_root())
        return true;

    if ((flag == MeshPacketFlag_t::ToNode || flag == MeshPacketFlag_t::FromIP) &&
        !isDestinationCurrentDevice(header))
        return true;

    if (flag == MeshPacketFlag_t::ToIP)
        return true;

    return false;
}

bool MeshDataExchangeModule::isToExternalIP(const int flag)
{
    if (flag & MESH_DATA_TODS)
        return true;

    return false;
}

bool MeshDataExchangeModule::isFromNodeToNode(const int flag)
{
    if (flag & MESH_DATA_P2P)
        return true;

    return false;
}

bool MeshDataExchangeModule::isFromExternalIPToNode(const int flag)
{
    if (flag & MESH_DATA_FROMDS)
        return true;

    return false;
}