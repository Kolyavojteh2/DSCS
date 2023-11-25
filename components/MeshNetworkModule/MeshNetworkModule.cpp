#include "MeshNetworkModule.h"
#include "MeshDataExchangeModule.h"
#include "WifiModule.h"
#include "DSSProtocolHandler.h"
#include "TCPClient.h"

#include <esp_netif.h>

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"

#ifndef MAC_ADDRESS_LENGTH
#define MAC_ADDRESS_LENGTH (6)
#endif // MAC_ADDRESS_LENGTH

// TODO: add this into Kconfig
const double MESH_ROOT_ELECTION_THRESHOLD = 0.9;
static const uint8_t meshId[6] = {0x77, 0x77, 0x77, 0x77, 0x77, 0x77};

MeshNetworkModule &MeshNetworkModule::getInstance()
{
    static MeshNetworkModule instance;

    return instance;
}

MeshNetworkModule::MeshNetworkModule()
{
    // mesh initialization
    ESP_ERROR_CHECK(esp_mesh_init());

    // register handlers
    ESP_ERROR_CHECK(esp_event_handler_register(MESH_EVENT, ESP_EVENT_ANY_ID, &meshEventHandler, NULL));

    // Base mesh configuration
    initBaseConfiguration(meshId);

    // Mesh shared configuration
    initSharedSettings();

    // Mesh start
    ESP_ERROR_CHECK(esp_mesh_start());

    // Update AP MAC
    esp_netif_t *esp_netif = esp_netif_next(NULL);
    size_t netif_count = esp_netif_get_nr_of_ifs();
    for (size_t i = 0; i < netif_count; ++i)
    {
        const char *ifkey = esp_netif_get_ifkey(esp_netif);
        if (strcmp(ifkey, "WIFI_AP_DEF") == 0)
        {
            uint8_t mac[6];
            esp_netif_get_mac(esp_netif, mac);
            WifiModule::getInstance().setApMAC(mac);

            break;
        }

        esp_netif = esp_netif_next(esp_netif);
    }
}

void MeshNetworkModule::initBaseConfiguration(const uint8_t *mesh_id)
{
    mesh_cfg_t cfg = MESH_INIT_CONFIG_DEFAULT();
    // mesh ID
    memcpy((uint8_t *)&cfg.mesh_id.addr, mesh_id, 6);

    // router
    // mesh wifi channel
    cfg.channel = CONFIG_MESH_CHANNEL;

    // router: SSID and password
    cfg.router.ssid_len = strlen(CONFIG_ROUTER_SSID);
    memcpy((uint8_t *)&cfg.router.ssid, CONFIG_ROUTER_SSID, cfg.router.ssid_len);
    memcpy((uint8_t *)&cfg.router.password, CONFIG_ROUTER_PASSWORD, strlen(CONFIG_ROUTER_PASSWORD));

    // mesh softAP: max connections, non_mesh connections, mesh password
    ESP_ERROR_CHECK(esp_mesh_set_ap_authmode((wifi_auth_mode_t)CONFIG_MESH_AP_AUTHMODE));
    cfg.mesh_ap.max_connection = CONFIG_MESH_AP_CONNECTIONS;
    cfg.mesh_ap.nonmesh_max_connection = CONFIG_MESH_NON_MESH_AP_CONNECTIONS;
    memcpy((uint8_t *)&cfg.mesh_ap.password, CONFIG_MESH_PASSWORD, strlen(CONFIG_MESH_PASSWORD));

    // set configuration
    ESP_ERROR_CHECK(esp_mesh_set_config(&cfg));
}

void MeshNetworkModule::initSharedSettings()
{
    // Mesh topology settings
    ESP_ERROR_CHECK(esp_mesh_set_topology(MESH_TOPO_TREE));
    ESP_ERROR_CHECK(esp_mesh_set_max_layer(CONFIG_MESH_MAX_LAYER));
    ESP_ERROR_CHECK(esp_mesh_set_capacity_num(CONFIG_MESH_MAX_CAPACITY));
    ESP_ERROR_CHECK(esp_mesh_set_vote_percentage(MESH_ROOT_ELECTION_THRESHOLD));

    mesh_attempts_t attempts;
    attempts.scan = CONFIG_MESH_ELECTION_ROUNDS;
    attempts.vote = CONFIG_MESH_SELF_HEALING_ELECTIONS_ROUNDS;
    attempts.fail = CONFIG_MESH_PARRENT_RECONNECTION_TRIES;
    attempts.monitor_ie = CONFIG_MESH_MAXIMUM_NUMBER_OF_BEACON_FRAMES_WITH_UPDATED_MESH_IE;
    ESP_ERROR_CHECK(esp_mesh_set_attempts(&attempts));

    // Root node healing delay
    ESP_ERROR_CHECK(esp_mesh_set_root_healing_delay(CONFIG_MESH_ROOT_HEALING_DELAY));

    // Other root node settings
    ESP_ERROR_CHECK(esp_mesh_set_xon_qsize(CONFIG_MESH_ROOT_QUEUE_FOR_PACKETS_SIZE));

    // Mesh Additional SoftAP Settings
    ESP_ERROR_CHECK(esp_mesh_set_ap_assoc_expire(CONFIG_MESH_CHILD_DISASSOCIATION_DELAY));

    // Mesh Node-specific settings
    // -- NONE --
}

void MeshNetworkModule::initPS()
{
    // Disable mesh PS function
    ESP_ERROR_CHECK(esp_mesh_disable_ps());
}

void MeshNetworkModule::onMeshEventStarted(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ESP_LOGI(moduleTag, "<MESH_EVENT_MESH_STARTED>");
}

void MeshNetworkModule::onMeshEventStopped(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ESP_LOGI(moduleTag, "<MESH_EVENT_STOPPED>");

    // TODO: add reestablish connect if error
    // if (0)
    //{
    //    ESP_LOGI(moduleTag, "Trying to reestablish mesh network");
    //    ESP_ERROR_CHECK(esp_mesh_start());
    //}
}

void MeshNetworkModule::onMeshEventChildConnected(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    mesh_event_child_connected_t *child_connected = (mesh_event_child_connected_t *)event_data;
    ESP_LOGI(moduleTag, "<MESH_EVENT_CHILD_CONNECTED>aid:%d, " MACSTR "", child_connected->aid, MAC2STR(child_connected->mac));
}

void MeshNetworkModule::onMeshEventChildDisconnected(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    mesh_event_child_disconnected_t *child_disconnected = (mesh_event_child_disconnected_t *)event_data;
    ESP_LOGI(moduleTag, "<MESH_EVENT_CHILD_DISCONNECTED>aid:%d, " MACSTR "", child_disconnected->aid, MAC2STR(child_disconnected->mac));
}

void MeshNetworkModule::onMeshEventRoutingTableAdd(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    mesh_event_routing_table_change_t *routing_table = (mesh_event_routing_table_change_t *)event_data;
    ESP_LOGW(moduleTag, "<MESH_EVENT_ROUTING_TABLE_ADD>add %d, new:%d", routing_table->rt_size_change, routing_table->rt_size_new);
}

void MeshNetworkModule::onMeshEventRoutingTableRemove(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    mesh_event_routing_table_change_t *routing_table = (mesh_event_routing_table_change_t *)event_data;
    ESP_LOGW(moduleTag, "<MESH_EVENT_ROUTING_TABLE_REMOVE>remove %d, new:%d",
             routing_table->rt_size_change, routing_table->rt_size_new);
}

void MeshNetworkModule::onMeshEventParentNotFound(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    mesh_event_no_parent_found_t *no_parent = (mesh_event_no_parent_found_t *)event_data;
    ESP_LOGI(moduleTag, "<MESH_EVENT_NO_PARENT_FOUND>scan times:%d", no_parent->scan_times);
}

void MeshNetworkModule::onMeshEventParentConnected(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    mesh_event_connected_t *connected = (mesh_event_connected_t *)event_data;
    mesh_addr_t parrentMAC;
    esp_mesh_get_parent_bssid(&parrentMAC);

    const char *current_type = esp_mesh_is_root() ? "<ROOT>" : "";
    ESP_LOGI(moduleTag, "<MESH_EVENT_PARENT_CONNECTED> parent:" MACSTR "%s, duty:%d",
             MAC2STR(parrentMAC.addr), current_type, connected->duty);

    std::vector<uint8_t> parent(parrentMAC.addr, parrentMAC.addr + MAC_ADDRESS_LENGTH);
    MeshNetworkModule::getInstance().setParentAddress(parent);

    if (esp_mesh_is_root())
    {
        // TODO: clean code
        esp_netif_t *esp_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");

        // TODO: add DHCP client actions in other places
        esp_netif_dhcp_status_t status;
        esp_err_t err = esp_netif_dhcpc_get_status(esp_netif, &status);
        if (err != ESP_OK)
        {
            // TODO: add error handle
        }
        if (status != ESP_NETIF_DHCP_STARTED)
            ESP_ERROR_CHECK(esp_netif_dhcpc_start(esp_netif));

        // TODO: add DNS getting info
    }

    MeshDataExchangeModule::getInstance().startReceiving();

    MeshDataExchangeModule::getInstance().createReceiveMeshTask();
}

void MeshNetworkModule::onMeshEventParentDisconnected(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    mesh_event_disconnected_t *disconnected = (mesh_event_disconnected_t *)event_data;
    ESP_LOGI(moduleTag, "<MESH_EVENT_PARENT_DISCONNECTED>reason:%d", disconnected->reason);

    MeshDataExchangeModule::getInstance().stopReceiving();
}

void MeshNetworkModule::onMeshEventLayerChange(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    const char *current_type = esp_mesh_is_root() ? "<ROOT>" : "";
    ESP_LOGI(moduleTag, "<MESH_EVENT_LAYER_CHANGE>layer type: %s", current_type);
}

void MeshNetworkModule::onMeshEventRootAddress(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    mesh_event_root_address_t *root_addr = (mesh_event_root_address_t *)event_data;
    ESP_LOGI(moduleTag, "<MESH_EVENT_ROOT_ADDRESS>root address:" MACSTR "", MAC2STR(root_addr->addr));

    std::vector<uint8_t> root(std::vector<uint8_t>(root_addr->addr, root_addr->addr + MAC_ADDRESS_LENGTH));
    MeshNetworkModule::getInstance().setRootAddress(root);
}

void MeshNetworkModule::onMeshEventVoteStarted(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    mesh_event_vote_started_t *vote_started = (mesh_event_vote_started_t *)event_data;
    ESP_LOGI(moduleTag, "<MESH_EVENT_VOTE_STARTED>attempts:%d, reason:%d, rc_addr:" MACSTR "",
             vote_started->attempts, vote_started->reason, MAC2STR(vote_started->rc_addr.addr));
}

void MeshNetworkModule::onMeshEventVoteStopped(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ESP_LOGI(moduleTag, "<MESH_EVENT_VOTE_STOPPED>");
}

void MeshNetworkModule::onMeshEventRootSwitchReq(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    mesh_event_root_switch_req_t *switch_req = (mesh_event_root_switch_req_t *)event_data;
    ESP_LOGI(moduleTag, "<MESH_EVENT_ROOT_SWITCH_REQ>reason:%d, rc_addr:" MACSTR "",
             switch_req->reason, MAC2STR(switch_req->rc_addr.addr));
}

void MeshNetworkModule::onMeshEventRootSwitchAck(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ESP_LOGI(moduleTag, "<MESH_EVENT_ROOT_SWITCH_ACK>");
}

void MeshNetworkModule::onMeshEventToDS(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    mesh_event_toDS_state_t *toDs_state = (mesh_event_toDS_state_t *)event_data;
    ESP_LOGI(moduleTag, "<MESH_EVENT_TODS_REACHABLE>state:%d", *toDs_state);

    // If non-Root, return
    if (esp_mesh_is_root())
    {
        TCPClient::getInstance().closeSocket();

        if (*toDs_state == MESH_TODS_REACHABLE)
        {
            TCPClient::getInstance().tryConnect();
            MeshDataExchangeModule::getInstance().createReceiveIPTask();
        }
    }
    else
    {
        DSSProtocolHandler::bootstrapSend();
    }
}

void MeshNetworkModule::onMeshEventRootFixed(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    mesh_event_root_fixed_t *root_fixed = (mesh_event_root_fixed_t *)event_data;
    ESP_LOGI(moduleTag, "<MESH_EVENT_ROOT_FIXED>%s", root_fixed->is_fixed ? "fixed" : "not fixed");
}

void MeshNetworkModule::onMeshEventRootAskedYield(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    mesh_event_root_conflict_t *root_conflict = (mesh_event_root_conflict_t *)event_data;
    ESP_LOGI(moduleTag, "<MESH_EVENT_ROOT_ASKED_YIELD>" MACSTR ", rssi:%d, capacity:%d",
             MAC2STR(root_conflict->addr), root_conflict->rssi, root_conflict->capacity);
}

void MeshNetworkModule::onMeshEventChannelSwitch(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    mesh_event_channel_switch_t *channel_switch = (mesh_event_channel_switch_t *)event_data;
    ESP_LOGI(moduleTag, "<MESH_EVENT_CHANNEL_SWITCH>new channel:%d", channel_switch->channel);
}

void MeshNetworkModule::onMeshEventScanDone(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    mesh_event_scan_done_t *scan_done = (mesh_event_scan_done_t *)event_data;
    ESP_LOGI(moduleTag, "<MESH_EVENT_SCAN_DONE>number:%d", scan_done->number);
}

void MeshNetworkModule::onMeshEventNetworkState(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    mesh_event_network_state_t *network_state = (mesh_event_network_state_t *)event_data;
    ESP_LOGI(moduleTag, "<MESH_EVENT_NETWORK_STATE>is_rootless:%d", network_state->is_rootless);
}

void MeshNetworkModule::onMeshEventStopReconnection(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ESP_LOGI(moduleTag, "<MESH_EVENT_STOP_RECONNECTION>");
}

void MeshNetworkModule::onMeshEventFindNetwork(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    mesh_event_find_network_t *find_network = (mesh_event_find_network_t *)event_data;
    ESP_LOGI(moduleTag, "<MESH_EVENT_FIND_NETWORK>new channel:%d, router BSSID:" MACSTR "", find_network->channel, MAC2STR(find_network->router_bssid));
}

void MeshNetworkModule::onMeshEventRouterSwitch(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    mesh_event_router_switch_t *router_switch = (mesh_event_router_switch_t *)event_data;
    ESP_LOGI(moduleTag, "<MESH_EVENT_ROUTER_SWITCH>new router:%s, channel:%d, " MACSTR "",
             router_switch->ssid, router_switch->channel, MAC2STR(router_switch->bssid));
}

void MeshNetworkModule::onMeshEventPSParentDuty(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    // TODO: check if this event is needing
    mesh_event_ps_duty_t *ps_duty = (mesh_event_ps_duty_t *)event_data;
    ESP_LOGI(moduleTag, "<MESH_EVENT_PS_PARENT_DUTY>duty:%d", ps_duty->duty);
}

void MeshNetworkModule::onMeshEventPSChildDuty(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    // TODO: check if this event is needing
    mesh_event_ps_duty_t *ps_duty = (mesh_event_ps_duty_t *)event_data;
    ESP_LOGI(moduleTag, "<MESH_EVENT_PS_CHILD_DUTY>cidx:%d, " MACSTR ", duty:%d", ps_duty->child_connected.aid - 1,
             MAC2STR(ps_duty->child_connected.mac), ps_duty->duty);
}

void MeshNetworkModule::onMeshEventUnknown(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ESP_LOGI(moduleTag, "unknown id:%ld", event_id);
}

void MeshNetworkModule::meshEventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case MESH_EVENT_STARTED:
        onMeshEventStarted(arg, event_base, event_id, event_data);
        break;

    case MESH_EVENT_STOPPED:
        onMeshEventStopped(arg, event_base, event_id, event_data);
        break;

    case MESH_EVENT_CHILD_CONNECTED:
        onMeshEventChildConnected(arg, event_base, event_id, event_data);
        break;

    case MESH_EVENT_CHILD_DISCONNECTED:
        onMeshEventChildDisconnected(arg, event_base, event_id, event_data);
        break;

    case MESH_EVENT_ROUTING_TABLE_ADD:
        onMeshEventRoutingTableAdd(arg, event_base, event_id, event_data);
        break;

    case MESH_EVENT_ROUTING_TABLE_REMOVE:
        onMeshEventRoutingTableRemove(arg, event_base, event_id, event_data);
        break;

    case MESH_EVENT_NO_PARENT_FOUND:
        onMeshEventParentNotFound(arg, event_base, event_id, event_data);
        break;

    case MESH_EVENT_PARENT_CONNECTED:
        onMeshEventParentConnected(arg, event_base, event_id, event_data);
        break;

    case MESH_EVENT_PARENT_DISCONNECTED:
        onMeshEventParentDisconnected(arg, event_base, event_id, event_data);
        break;

    case MESH_EVENT_LAYER_CHANGE:
        onMeshEventLayerChange(arg, event_base, event_id, event_data);
        break;

    case MESH_EVENT_ROOT_ADDRESS:
        onMeshEventRootAddress(arg, event_base, event_id, event_data);
        break;

    case MESH_EVENT_VOTE_STARTED:
        onMeshEventVoteStarted(arg, event_base, event_id, event_data);
        break;

    case MESH_EVENT_VOTE_STOPPED:
        onMeshEventVoteStopped(arg, event_base, event_id, event_data);
        break;

    case MESH_EVENT_ROOT_SWITCH_REQ:
        onMeshEventRootSwitchReq(arg, event_base, event_id, event_data);
        break;

    case MESH_EVENT_ROOT_SWITCH_ACK:
        onMeshEventRootSwitchAck(arg, event_base, event_id, event_data);
        break;

    case MESH_EVENT_TODS_STATE:
        onMeshEventToDS(arg, event_base, event_id, event_data);
        break;

    case MESH_EVENT_ROOT_FIXED:
        onMeshEventRootFixed(arg, event_base, event_id, event_data);
        break;

    case MESH_EVENT_ROOT_ASKED_YIELD:
        onMeshEventRootAskedYield(arg, event_base, event_id, event_data);
        break;

    case MESH_EVENT_CHANNEL_SWITCH:
        onMeshEventChannelSwitch(arg, event_base, event_id, event_data);
        break;

    case MESH_EVENT_SCAN_DONE:
        onMeshEventScanDone(arg, event_base, event_id, event_data);
        break;

    case MESH_EVENT_NETWORK_STATE:
        onMeshEventNetworkState(arg, event_base, event_id, event_data);
        break;

    case MESH_EVENT_STOP_RECONNECTION:
        onMeshEventStopReconnection(arg, event_base, event_id, event_data);
        break;

    case MESH_EVENT_FIND_NETWORK:
        onMeshEventFindNetwork(arg, event_base, event_id, event_data);
        break;

    case MESH_EVENT_ROUTER_SWITCH:
        onMeshEventRouterSwitch(arg, event_base, event_id, event_data);
        break;

    case MESH_EVENT_PS_PARENT_DUTY:
        onMeshEventPSParentDuty(arg, event_base, event_id, event_data);
        break;

    case MESH_EVENT_PS_CHILD_DUTY:
        onMeshEventPSChildDuty(arg, event_base, event_id, event_data);
        break;

    default:
        onMeshEventUnknown(arg, event_base, event_id, event_data);
        break;
    }
}

const std::vector<uint8_t> &MeshNetworkModule::getRootAddress() const
{
    return rootAddress;
}

void MeshNetworkModule::setRootAddress(const std::vector<uint8_t> &root)
{
    rootAddress = root;
}

const std::vector<uint8_t> &MeshNetworkModule::getParentAddress() const
{
    return parentAddress;
}

void MeshNetworkModule::setParentAddress(const std::vector<uint8_t> &parent)
{
    parentAddress = parent;
}