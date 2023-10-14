#ifndef MESH_NETWORK_MODULE_H
#define MESH_NETWORK_MODULE_H

#include <vector>

#include <esp_mac.h>
#include <esp_event.h>

#include <esp_mesh.h>
#include <esp_mesh_internal.h>

#include <esp_log.h>

class MeshNetworkModule
{
public:
    static MeshNetworkModule &getInstance(void);

    const std::vector<uint8_t> &getRootAddress() const;
    void setRootAddress(const std::vector<uint8_t> &root);

    const std::vector<uint8_t> &getParentAddress() const;
    void setParentAddress(const std::vector<uint8_t> &parent);

    int getSocket() const;

private:
    static constexpr const char *moduleTag = "mesh";

    MeshNetworkModule();
    MeshNetworkModule(const MeshNetworkModule &) = delete;
    MeshNetworkModule &operator=(const MeshNetworkModule &) = delete;

    void init();
    void initBaseConfiguration(const uint8_t *mesh_id);
    void initSharedSettings();
    void initPS();

    void getHostsIP(std::vector<mip_t> &hosts);

    static void meshEventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

    static void onMeshEventStarted(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void onMeshEventStopped(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void onMeshEventChildConnected(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void onMeshEventChildDisconnected(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void onMeshEventRoutingTableAdd(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void onMeshEventRoutingTableRemove(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void onMeshEventParentNotFound(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void onMeshEventParentConnected(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void onMeshEventParentDisconnected(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void onMeshEventLayerChange(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

    static void onMeshEventRootAddress(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void onMeshEventVoteStarted(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void onMeshEventVoteStopped(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void onMeshEventRootSwitchReq(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void onMeshEventRootSwitchAck(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void onMeshEventToDS(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void onMeshEventRootFixed(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void onMeshEventRootAskedYield(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void onMeshEventChannelSwitch(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void onMeshEventScanDone(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void onMeshEventNetworkState(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void onMeshEventStopReconnection(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void onMeshEventFindNetwork(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void onMeshEventRouterSwitch(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void onMeshEventPSParentDuty(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void onMeshEventPSChildDuty(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void onMeshEventUnknown(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

    int connectToServer();
    void disconnectFromServer();

    int socket_fd = 0;
    std::vector<uint8_t> rootAddress;
    std::vector<uint8_t> parentAddress;
};

#endif // MESH_NETWORK_MODULE_H