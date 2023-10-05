idf_component_register(
    SRCS
    "MeshNetworkModule.cpp"

    INCLUDE_DIRS
    "."

    REQUIRES
    "esp_event"
    "esp_wifi"

    "MeshDataExchangeModule"
)
