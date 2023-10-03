#include "MeshDataExchangeModule.h"

#include <esp_mesh.h>
#include <esp_log.h>

#define MAC_ADDRESS_LENGTH (6)

MeshDataExchangeModule &MeshDataExchangeModule::getInstance()
{
    static MeshDataExchangeModule instance;

    return instance;
}

MeshDataExchangeModule::MeshDataExchangeModule()
{
    m_receivingSemaphore = xSemaphoreCreateBinary();

    const int stackSize = 4096;
    const int priorityReceiving = 1;
    const int coreNumber = 1;
    xTaskCreatePinnedToCore(receiveTask, "receiveTask", stackSize, NULL, priorityReceiving, NULL, coreNumber);
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

void MeshDataExchangeModule::prepareSendPacket(mesh_data_t &sendPacket, const MeshExchangePacket_t &exchangePacket)
{
    // TODO: look if the memory of out, create allocate memory via new or malloc
    sendPacket.data = (unsigned char *)exchangePacket.data.data();
    sendPacket.size = exchangePacket.data.size();
    // TODO: add possibility for JSON and HTTP
    sendPacket.proto = MESH_PROTO_BIN;
    sendPacket.tos = MESH_TOS_P2P;
}

void MeshDataExchangeModule::prepareSendFlag(int &flag, const MeshExchangePacket_t &exchangePacket)
{
    if (exchangePacket.nonBlock)
        flag += MESH_DATA_NONBLOCK;
    if (exchangePacket.nonCritical)
        flag += MESH_DATA_DROP;
}

esp_err_t MeshDataExchangeModule::sendToRoot(const MeshExchangePacket_t &packet)
{
    esp_err_t ret = checkData(packet.data);
    if (ret)
        return ret;

    mesh_data_t dataDes;
    prepareSendPacket(dataDes, packet);

    int sendFlag = 0;
    prepareSendFlag(sendFlag, packet);

    // To root specific code
    ret = esp_mesh_send(NULL, &dataDes, sendFlag, NULL, 0);
    if (ret)
    {
        ESP_LOGE(moduleTag, "The message doesn't sent. Error code: %d", ret);
        // TODO: add hander with different errors and may be retry mechanism
    }

    return ret;
}

esp_err_t MeshDataExchangeModule::sendToNode(const mesh_addr_t *const dest, const MeshExchangePacket_t &packet)
{
    esp_err_t ret = checkData(packet.data);
    if (ret)
        return ret;

    mesh_data_t dataDes;
    prepareSendPacket(dataDes, packet);

    int sendFlag = 0;
    prepareSendFlag(sendFlag, packet);

    // To node specific code
    sendFlag += MESH_DATA_P2P;
    ret = esp_mesh_send(dest, &dataDes, sendFlag, NULL, 0);
    if (ret)
    {
        ESP_LOGE(moduleTag, "The message doesn't sent. Error code: %d", ret);
        // TODO: add hander with different errors and may be retry mechanism
    }

    return ret;
}

esp_err_t MeshDataExchangeModule::sendToNodeFromIP(const mesh_addr_t *const dest, const mesh_addr_t *const source, const MeshExchangePacket_t &packet)
{
    esp_err_t ret = checkData(packet.data);
    if (ret)
        return ret;

    mesh_data_t dataDes;
    prepareSendPacket(dataDes, packet);

    int sendFlag = 0;
    prepareSendFlag(sendFlag, packet);

    // To external IP specific code
    sendFlag += MESH_DATA_P2P;
    sendFlag += MESH_DATA_FROMDS;

    mesh_opt_t sendOpt;

    // TODO: check if the sendOpt.val can be the same as source pointer
    sendOpt.type = MESH_OPT_RECV_DS_ADDR;
    sendOpt.val = (uint8_t *)source;
    sendOpt.len = MAC_ADDRESS_LENGTH;

    ret = esp_mesh_send(dest, &dataDes, sendFlag, &sendOpt, 1);
    if (ret)
    {
        ESP_LOGE(moduleTag, "The message doesn't sent. Error code: %d", ret);
        // TODO: add hander with different errors and may be retry mechanism
    }

    return ret;
}

esp_err_t MeshDataExchangeModule::sendToIP(const mesh_addr_t *const dest, const MeshExchangePacket_t &packet)
{
    esp_err_t ret = checkData(packet.data);
    if (ret)
        return ret;

    mesh_data_t dataDes;
    prepareSendPacket(dataDes, packet);

    int sendFlag = 0;
    prepareSendFlag(sendFlag, packet);

    // To external IP specific code
    sendFlag += MESH_DATA_TODS;
    ret = esp_mesh_send(dest, &dataDes, sendFlag, NULL, 0);
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
    xSemaphoreGive(m_receivingSemaphore);
}

void MeshDataExchangeModule::stopReceiving(void)
{
    m_receivingState = false;
}

void MeshDataExchangeModule::receiveTask(void * /*unused*/)
{
    esp_err_t recvRet;            // Used to store the result of the recv()
    uint8_t recvBuffer[MESH_MTU]; // Receive Buffer (MESH_MTU = 1500 bytes)
    mesh_addr_t source = {0};     // Received packet's source address
    int recvFlag = 0;             // Received packet's flag
    mesh_opt_t recvOpt;           // Received packet's additional option
    uint8_t optVal[50];           // Received packet's additional option's value

    recvOpt.val = optVal;

    mesh_data_t dataDes = {.data = recvBuffer, .size = MESH_MTU, .proto = MESH_PROTO_BIN, .tos = MESH_TOS_P2P}; // Received packet's data descriptor

    while (1)
    {
        // Check the receiving state
        if (MeshDataExchangeModule::getInstance().m_receivingState == false)
            xSemaphoreTake(MeshDataExchangeModule::getInstance().m_receivingSemaphore, portMAX_DELAY);

        dataDes.size = MESH_MTU;
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
        ESP_LOGI(moduleTag, "Received data: %s", dataDes.data);

        // Receive mechanism handlers
        if (recvFlag & MESH_DATA_TODS)
        {
            ESP_LOGI(moduleTag, "Destination: toDS");
            // from node to external IP
            // TODO: add mechanism for handling packets from a node to an external IP
        }

        if (recvFlag & MESH_DATA_FROMDS || recvOpt.type == MESH_OPT_RECV_DS_ADDR)
        {
            ESP_LOGI(moduleTag, "Destination: fromDS");
            // from external IP
            // TODO: add mechanism for handling packets from an external IP
        }

        if (recvFlag & MESH_DATA_P2P)
        {
            ESP_LOGI(moduleTag, "Destination: P2P");
            // from node
            // TODO: add mechanism for handling packets from a node
        }
    }
}