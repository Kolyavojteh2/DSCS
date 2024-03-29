#ifndef MESH_DATA_EXCHANGE_MODULE_H
#define MESH_DATA_EXCHANGE_MODULE_H

#include <vector>
#include <inttypes.h>

#include <esp_mesh.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "PacketHandleMechanism.h"

#include "DSS_Protocol.h"

#define RECEIVE_MESH_TASK_NAME "receiveMeshTask"
#define RECEIVE_IP_TASK_NAME "receiveIPTask"
#define CLEANER_TASK_NAME "cleanerTask"

#define RECEIVE_TASK_STACK_SIZE (8192)
#define RECEIVE_TASK_PRIORITY (1)
#define RECEIVE_TASK_CORE_NUMBER (1)

enum MeshPacketFlag_t
{
    ToRoot = 0,
    ToNode = MESH_DATA_P2P,
    ToIP = MESH_DATA_TODS,
    FromIP = MESH_DATA_P2P | MESH_DATA_FROMDS,
};

class MeshDataExchangeModule
{
public:
    // Singleton
    static MeshDataExchangeModule &getInstance();

    // Send Functions
    static esp_err_t sendToRoot(const std::vector<uint8_t> &bin);
    static esp_err_t sendToNode(const std::vector<uint8_t> &bin);
    static esp_err_t sendToIP(const std::vector<uint8_t> &bin);
    static esp_err_t sendFromIP(const std::vector<uint8_t> &bin);

    // Receive control
    void startReceiving(void);
    void stopReceiving(void);

    static PacketHandleMechanism m_mechanismToExternalIP;
    static PacketHandleMechanism m_mechanismFromExternalIP;
    static PacketHandleMechanism m_mechanismToNode;

    static PacketHandleMechanism m_mechanismPacketHandlers;

    void createReceiveIPTask();
    void createReceiveMeshTask();

    TaskHandle_t getReceiveIPTaskHandle() const;
    TaskHandle_t getReceiveMeshTaskHandle() const;

private:
    static constexpr const char *moduleTag = "MDEM";

    MeshDataExchangeModule();
    MeshDataExchangeModule(const MeshDataExchangeModule &) = delete;
    MeshDataExchangeModule &operator=(const MeshDataExchangeModule &) = delete;

    static esp_err_t sendToIPAsNode(const std::vector<uint8_t> &bin);
    static esp_err_t sendToIPAsRoot(const std::vector<uint8_t> &bin);

    static bool checkData(const std::vector<uint8_t> &data);

    static void prepareSendPacket(mesh_data_t &sendPacket, const std::vector<uint8_t> &bin);

    // Receive task
    static void receiveMeshTask(void *);
    static void receiveIPTask(void *);

    static void analyzeAndProcessData(std::vector<uint8_t> bin, const int flag);

    static void handleReceivedData(const std::vector<uint8_t> bin, const int flag);
    static void retransmitReceivedData(const std::vector<uint8_t> bin, const int flag);

    static bool isNeedToHandle(const DSS_Protocol_t &header, const int flag);
    static bool isNeedToRetransmit(const DSS_Protocol_t &header, const int flag);

    static bool isToExternalIP(const int flag);
    static bool isFromNodeToNode(const int flag);
    static bool isFromExternalIPToNode(const int flag);

    static bool isDestinationCurrentDevice(const DSS_Protocol_t &header);
    static bool isBroadcast(const DSS_Protocol_t &header);

    // Receive control
    bool m_receivingState = false;

    // Task handles
    TaskHandle_t m_receiveIPTaskHandle = NULL;
    TaskHandle_t m_receiveMeshTaskHandle = NULL;
};

#endif // MESH_DATA_EXCHANGE_MODULE_H