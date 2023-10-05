
// #include "esp_netif_types.h"
#include <esp_netif.h>
// #include <esp_netif/lwip/esp_netif_lwip_internal.h>

#include <esp_event.h>
#include <lwip/sockets.h>
#include <esp_mac.h>
#include <esp_mesh_internal.h>
#include <esp_mesh.h>
#include <esp_wifi.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <esp_log.h>

#include "WifiModule.h"

WifiModule::WifiModule()
{
    ESP_LOGI(moduleTag, "Constructor.");
    init();
}

void WifiModule::init()
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_netif_create_default_wifi_mesh_netifs(&wifi_netif_sta, NULL));

    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&config));

    // Disable DHCP server and client
    esp_netif_t *esp_netif = esp_netif_next(NULL);
    size_t netif_count = esp_netif_get_nr_of_ifs();

    for (size_t i = 0; i < netif_count; ++i)
    {
        esp_netif_dhcp_status_t status;

        // stop DHCP client
        esp_err_t err = esp_netif_dhcpc_get_status(esp_netif, &status);
        if (err != ESP_OK)
        {
            // TODO: add error handle
            esp_netif = esp_netif_next(esp_netif);
            continue;
        }
        if (status == ESP_NETIF_DHCP_STARTED)
            ESP_ERROR_CHECK(esp_netif_dhcpc_stop(esp_netif));

        // stop DHCP server
        err = esp_netif_dhcps_get_status(esp_netif, &status);
        if (err != ESP_OK)
        {
            // TODO: add error handle
            esp_netif = esp_netif_next(esp_netif);
            continue;
        }
        if (status == ESP_NETIF_DHCP_STARTED)
            ESP_ERROR_CHECK(esp_netif_dhcps_stop(esp_netif));

        /*
        // stop DHCP client
        if (esp_netif->dhcpc_status == ESP_NETIF_DHCP_STARTED)
            ESP_ERROR_CHECK(esp_netif_dhcpc_stop(esp_netif));

        // stop DHCP server
        if (esp_netif->dhcps_status == ESP_NETIF_DHCP_STARTED)
            ESP_ERROR_CHECK(esp_netif_dhcps_stop(esp_netif));

        esp_netif = esp_netif_next(esp_netif);
        */
        esp_netif = esp_netif_next(esp_netif);
    }

    // TODO: look at the name IP instead WIFI
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &this->ip_event_handler, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void WifiModule::on_event_STA_got_IP(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(moduleTag, "<IP_EVENT_STA_GOT_IP>IP:" IPSTR, IP2STR(&event->ip_info.ip));

    // TODO: start from here the mesh root driver
    // Inform that external DS is reachable
    ESP_ERROR_CHECK(esp_mesh_post_toDS_state(true));
}

void WifiModule::on_event_STA_lost_IP(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ESP_LOGI(moduleTag, "<IP_EVENT_STA_LOST_IP>");

    if (esp_mesh_is_root())
        ESP_ERROR_CHECK(esp_mesh_post_toDS_state(false));
}

void WifiModule::ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    // TODO: add other events
    switch (event_id)
    {
    case IP_EVENT_STA_GOT_IP:
        on_event_STA_got_IP(arg, event_base, event_id, event_data);
        break;

    case IP_EVENT_STA_LOST_IP:
        on_event_STA_lost_IP(arg, event_base, event_id, event_data);
        break;

    default:
        // TODO: add unknown event
        break;
    }
}

void WifiModule::reset()
{
    // TODO: add reset logic
    ESP_LOGI(moduleTag, "Wifi module was reset.");
}