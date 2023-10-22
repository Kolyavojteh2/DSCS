idf_component_register(
    SRCS
    "DistributedSensorSystemClient.cpp"
    "DSSProtocolHandler.cpp"

    INCLUDE_DIRS
    "."

    REQUIRES
    "PacketHandleMechanism"
    "DSS_Protocol"
    "MeshDataExchangeModule"
    "WifiModule"
    "MeshNetworkModule"
    "ConfigurationManager"
    "TimeManager"

    PRIV_REQUIRES
    "esp_wifi"
)
