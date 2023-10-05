idf_component_register(
    SRCS
    "MeshDataExchangeModule.cpp"

    INCLUDE_DIRS
    "."

    PRIV_REQUIRES
    "freertos"
    "esp_wifi"
)
