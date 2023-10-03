#include <iostream>

#include "FlashStorage/FlashStorage.h"
#include "MeshNetworkModule/MeshNetworkModule.h"
#include "WifiModule/WifiModule.h"
#include "MeshDataExchangeModule/MeshDataExchangeModule.h"

#include "esp_netif.h"

// Отримання та відображення інформації про esp_netif_t
void display_netif_info(esp_netif_t *netif)
{
    printf("Name: %s\n", esp_netif_get_desc(netif));
    printf("ifkey: %s\n", esp_netif_get_ifkey(netif));
    uint8_t mac[6] = {0, 0, 0, 0, 0, 0};
    esp_netif_get_mac(netif, mac);
    printf("MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    const char *hostname = NULL;
    esp_netif_get_hostname(netif, &hostname);
    if (hostname)
        printf("hostname: %s\n", hostname);

    esp_netif_ip_info_t ip_info;
    esp_netif_get_ip_info(netif, &ip_info);
    printf("IP Address: %s\n", ip4addr_ntoa((const ip4_addr_t *)&ip_info.ip));
    printf("Subnet Mask: %s\n", ip4addr_ntoa((const ip4_addr_t *)&ip_info.netmask));
    printf("Gateway: %s\n", ip4addr_ntoa((const ip4_addr_t *)&ip_info.gw));
}

extern "C" void app_main(void)
{
    FlashStorage flash;
    WifiModule wifi;
    MeshNetworkModule meshNetwork;
    MeshDataExchangeModule::getInstance();

    vTaskDelay(pdMS_TO_TICKS(10000));

    esp_netif_t *netif = esp_netif_next(NULL);
    while (netif)
    {
        display_netif_info(netif);
        netif = esp_netif_next(netif);
    }

    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
