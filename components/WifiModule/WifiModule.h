#ifndef WIFI_MODULE_H
#define WIFI_MODULE_H

#include <esp_netif.h>
#include <esp_event.h>

class WifiModule
{
public:
    static WifiModule &getInstance(void);

    void reset();

    // TODO: add clearing of the MAC after stopping or lossing connection
    void setStaMAC(const uint8_t *mac);
    void setApMAC(const uint8_t *mac);

    uint8_t *getStaMAC(void);
    uint8_t *getApMAC(void);

private:
    static constexpr const char *moduleTag = "wifi";

    WifiModule();
    WifiModule(const WifiModule &) = delete;
    WifiModule &operator=(const WifiModule &) = delete;

    void init();
    static void ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

    static void on_event_STA_got_IP(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void on_event_STA_lost_IP(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

    esp_netif_t *wifi_netif_sta = NULL;

    uint8_t m_staMAC[6];
    uint8_t m_apMAC[6];
};

#endif // WIFI_MODULE_H