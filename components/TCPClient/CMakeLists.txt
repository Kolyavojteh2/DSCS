idf_component_register(
    SRCS
    "TCPClient.cpp"

    INCLUDE_DIRS
    "."

    PRIV_REQUIRES
    "esp_system"
    "MeshDataExchangeModule"
)
