#ifndef WIFI_MODULE_H
#define WIFI_MODULE_H

#include <esp_netif.h>
#include <esp_event.h>

class WifiModule
{
public:
    WifiModule();

    void reset();

private:
    static constexpr const char *moduleTag = "wifi";

    void init();
    static void ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

    static void on_event_STA_got_IP(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void on_event_STA_lost_IP(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

    esp_netif_t *wifi_netif_sta = NULL;
};

#endif // WIFI_MODULE_H