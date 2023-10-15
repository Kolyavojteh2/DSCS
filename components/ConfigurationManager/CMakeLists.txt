idf_component_register(
    SRCS
    "ConfigurationManager.cpp"

    INCLUDE_DIRS
    "."

    REQUIRES
    "FlashStorage"

    PRIV_REQUIRES
    "esp_system"
)
