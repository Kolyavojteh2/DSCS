idf_component_register(
    SRCS
    "SensorManager.cpp"

    "SensorModules/BaseSensorModule.cpp"
    "SensorModules/SI1145/SI1145.cpp"
    "SensorModules/HDC1080/HDC1080.cpp"
    "SensorModules/CCS811/CCS811.cpp"
    "SensorModules/HeapMonitor/HeapMonitor.cpp"

    INCLUDE_DIRS
    "."

    REQUIRES
    "I2C_Peripheral"
    "TimeManager"
    "esp_system"
    "freertos"
)
