idf_component_register(SRCS
    "main.cpp"

    INCLUDE_DIRS
    "."

    REQUIRES
    "FlashStorage"
    "MeshDataExchangeModule"
    "MeshNetworkModule"
    "WifiModule"
    "DistributedSensorSystemClient"
    "TimeManager"
    "SensorManager"
)
