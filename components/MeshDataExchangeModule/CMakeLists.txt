idf_component_register(
    SRCS
    "MeshDataExchangeModule.cpp"

    INCLUDE_DIRS
    "."

    REQUIRES
    "PacketHandleMechanism"
    "DSS_Protocol"
    "WifiModule"
    "MeshNetworkModule"

    PRIV_REQUIRES
    "freertos"
    "esp_wifi"
)
