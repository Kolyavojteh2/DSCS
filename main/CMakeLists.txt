# add_subdirectory(components/FlashStorage)
# add_subdirectory(components/MeshDataExchangeModule)
# add_subdirectory(components/MeshNetworkModule)
# add_subdirectory(components/WifiModule)

idf_component_register(SRCS
    "main.cpp"

    # "WifiModule/WifiModule.cpp"
    # "MeshNetworkModule/MeshNetworkModule.cpp"
    # "FlashStorage/FlashStorage.cpp"
    # "MeshDataExchangeModule/MeshDataExchangeModule.cpp"

    # "ExchangeProtocol/ExchangeProtocol.cpp"
    # "ExchangeProtocol/utils/ConvertBinStruct.cpp"
    # "ExchangeProtocol/packets/AliveNodePackets.cpp"
    # "ExchangeProtocol/packets/ConfigurationPackets.cpp"
    # "ExchangeProtocol/packets/ControlPackets.cpp"
    INCLUDE_DIRS "."

    REQUIRES
    "FlashStorage"
    "MeshDataExchangeModule"
    "MeshNetworkModule"
    "WifiModule"
)
