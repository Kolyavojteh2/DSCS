#include "MeshDataExchangeModule.h"
#include "MeshNetworkModule.h"
#include "WifiModule.h"
#include "DSS_Protocol.h"
#include "TCPClient.h"

#include <esp_mesh.h>
#include <esp_log.h>

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"

#include <vector>

#ifndef RETRY_SEND_COUNT
#define RETRY_SEND_COUNT (5)
#endif // RETRY_SEND_COUNT

#ifndef RETRY_DELAY_BETWEEN_SENDING
#define RETRY_DELAY_BETWEEN_SENDING (2000)
#endif // RETRY_DELAY_BETWEEN_SENDING

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

MeshDataExchangeModule::MeshDataExchangeModule() {}

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
    int socket_fd;

    // Get socket
    for (unsigned int tryNumber = 1; tryNumber <= RETRY_SEND_COUNT; ++tryNumber)
    {
        // socket_fd = MeshNetworkModule::getInstance().getSocket();
        socket_fd = TCPClient::getInstance().getSocket();
        if (socket_fd <= 0)
            ESP_LOGE(moduleTag, "Got bad socket. Try number: %u. socket_fd: %d", tryNumber, socket_fd);
        else
            break;

        vTaskDelay(pdMS_TO_TICKS(RETRY_DELAY_BETWEEN_SENDING));
        if (tryNumber == RETRY_SEND_COUNT)
            return ESP_FAIL;
    }

    // Send data
    for (unsigned int tryNumber = 1; tryNumber <= RETRY_SEND_COUNT; ++tryNumber)
    {
        int sentBytes = write(socket_fd, bin.data(), bin.size());
        if (sentBytes != bin.size())
            ESP_LOGE(moduleTag, "TCP send(write) error. Try number: %u. Sent bytes: %d. Packet size: %d", tryNumber, sentBytes, bin.size());
        else
            break;

        vTaskDelay(pdMS_TO_TICKS(RETRY_DELAY_BETWEEN_SENDING));
        if (tryNumber == RETRY_SEND_COUNT)
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
}

void MeshDataExchangeModule::stopReceiving(void)
{
    m_receivingState = false;
}

void MeshDataExchangeModule::receiveIPTask(void * /*unused*/)
{
    ESP_LOGI(moduleTag, "Receiving IP started.");

    uint8_t recvBuffer[MESH_MTU];
    ssize_t gotBytes = 0;

    while (1)
    {
        // Check the receiving state
        if (MeshDataExchangeModule::getInstance().m_receivingState == false)
        {
            vTaskSuspend(NULL);
            continue;
        }

        // int socket_fd = MeshNetworkModule::getInstance().getSocket();
        int socket_fd = TCPClient::getInstance().getSocket();

        // Check if the socket_fd is valid
        if (socket_fd <= 0)
        {
            vTaskSuspend(NULL);
            continue;
        }

        // Add timeout for listening
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(socket_fd, &read_fds);

        // TODO: change timeout
        struct timeval timeout;
        timeout.tv_sec = 5; // in seconds
        timeout.tv_usec = 0;

        int ready = select(socket_fd + 1, &read_fds, NULL, NULL, &timeout);
        if (ready == 0)
            continue;

        if (ready < 0)
        {
            ESP_LOGE(moduleTag, "select error.");
            continue;
        }

        gotBytes = read(socket_fd, recvBuffer, MESH_MTU);
        if (gotBytes <= 0)
        {
            int errnoValue = errno;
            // TODO: add handling errno errors
            // TODO: remove hardcode
            if (gotBytes < 0 && errnoValue == 128)
                TCPClient::getInstance().closeSocket();

            ESP_LOGW(moduleTag, "read() returns: %d", gotBytes);
            ESP_LOGW(moduleTag, "errno: %d", errnoValue);
            continue;
        }

        // TODO: check if it is needing
        ESP_LOGI(moduleTag, "Received data from IP: %s", recvBuffer);

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
        {
            vTaskSuspend(NULL);
            continue;
        }

        dataDes.size = MESH_MTU;
        // TODO: check if needing it
        recvOpt.type = 0;

        // Receive the packet
        recvRet = esp_mesh_recv(&source, &dataDes, portMAX_DELAY, &recvFlag, &recvOpt, 1);
        if (recvRet != ESP_OK)
        {
            // TODO: add error handling
            continue;
        }

        // TODO: check if it is needing
        ESP_LOGI(moduleTag, "Received data from MESH: %s", dataDes.data);

        std::vector<uint8_t> bin(dataDes.data, dataDes.data + dataDes.size);
        analyzeAndProcessData(bin, recvFlag);
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

void MeshDataExchangeModule::createReceiveIPTask()
{
    if (m_receiveIPTaskHandle)
        return;

    xTaskCreatePinnedToCore(MeshDataExchangeModule::receiveIPTask,
                            RECEIVE_IP_TASK_NAME,
                            RECEIVE_TASK_STACK_SIZE,
                            NULL,
                            RECEIVE_TASK_PRIORITY,
                            &m_receiveIPTaskHandle,
                            RECEIVE_TASK_CORE_NUMBER);
}

void MeshDataExchangeModule::createReceiveMeshTask()
{
    if (m_receiveMeshTaskHandle)
        return;

    xTaskCreatePinnedToCore(MeshDataExchangeModule::receiveMeshTask,
                            RECEIVE_MESH_TASK_NAME,
                            RECEIVE_TASK_STACK_SIZE,
                            NULL,
                            RECEIVE_TASK_PRIORITY,
                            &m_receiveMeshTaskHandle,
                            RECEIVE_TASK_CORE_NUMBER);
}

TaskHandle_t MeshDataExchangeModule::getReceiveIPTaskHandle() const
{
    return m_receiveIPTaskHandle;
}
TaskHandle_t MeshDataExchangeModule::getReceiveMeshTaskHandle() const
{
    return m_receiveMeshTaskHandle;
}
