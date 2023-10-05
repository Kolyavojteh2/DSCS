idf_component_register(
    SRCS
    "WifiModule.cpp"

    INCLUDE_DIRS
    "."

    PRIV_REQUIRES
    "esp_netif"
    "esp_wifi"
    "esp_system"
    "nvs_flash"
)
