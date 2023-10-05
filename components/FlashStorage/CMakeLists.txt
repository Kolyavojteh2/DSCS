idf_component_register(
    SRCS
    "FlashStorage.cpp"

    INCLUDE_DIRS
    "."

    PRIV_REQUIRES
    "nvs_flash"
    "esp_system"
)
