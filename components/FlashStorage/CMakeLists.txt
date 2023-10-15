idf_component_register(
    SRCS
    "FlashStorage.cpp"

    INCLUDE_DIRS
    "."

    REQUIRES
    "nvs_flash"

    PRIV_REQUIRES
    "esp_system"
)
