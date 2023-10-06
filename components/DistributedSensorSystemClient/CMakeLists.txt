idf_component_register(
    SRCS
    "DistributedSensorSystemClient.cpp"
    "ExchangeProtocolHandler.cpp"

    INCLUDE_DIRS
    "."

    REQUIRES
    "PacketHandleMechanism"
    "ExchangeProtocol"
    "MeshDataExchangeModule"
    "WifiModule"

    PRIV_REQUIRES
    "esp_wifi"
)
