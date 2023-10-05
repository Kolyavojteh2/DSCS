#ifndef MESH_DATA_EXCHANGE_MODULE_H
#define MESH_DATA_EXCHANGE_MODULE_H

#include <vector>
#include <inttypes.h>

#include <esp_mesh.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include "PacketHandleMechanism.h"

struct MeshExchangePacket_t
{
    std::vector<uint8_t> data;
    bool nonBlock = false;
    bool nonCritical = false;
};

class MeshDataExchangeModule
{
public:
    // Singleton
    static MeshDataExchangeModule &getInstance();

    // Send functions
    static esp_err_t sendToRoot(const MeshExchangePacket_t &packet);
    // TODO: look at the dest, it can be a const mesh_addr_t dest& and with source the same
    static esp_err_t sendToNode(const mesh_addr_t *const dest, const MeshExchangePacket_t &packet);
    static esp_err_t sendToNodeFromIP(const mesh_addr_t *const dest, const mesh_addr_t *const source, const MeshExchangePacket_t &packet);
    static esp_err_t sendToIP(const mesh_addr_t *const dest, const MeshExchangePacket_t &packet);

    // Receive control
    void startReceiving(void);
    void stopReceiving(void);

    static PacketHandleMechanism m_mechanismToExternalIP;
    static PacketHandleMechanism m_mechanismFromExternalIP;
    static PacketHandleMechanism m_mechanismToNode;

private:
    static constexpr const char *moduleTag = "MDEM";

    MeshDataExchangeModule();
    MeshDataExchangeModule(const MeshDataExchangeModule &) = delete;
    MeshDataExchangeModule &operator=(const MeshDataExchangeModule &) = delete;

    static bool checkData(const std::vector<uint8_t> &data);
    static void prepareSendPacket(mesh_data_t &sendPacket, const MeshExchangePacket_t &exchangePacket);
    static void prepareSendFlag(int &flag, const MeshExchangePacket_t &exchangePacket);

    // Receive task
    static void receiveTask(void *);

    // Receive control
    bool m_receivingState = false;
    SemaphoreHandle_t m_receivingSemaphore;
};

#endif // MESH_DATA_EXCHANGE_MODULE_H