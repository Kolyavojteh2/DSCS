#include "SensorManager.h"

#include <esp_err.h>
#include <esp_log.h>

#define MAX_DATA_PACKET_SIZE 1000

SensorManager::SensorManager()
{
    ESP_LOGI(moduleTag, "initialized");
}

SensorManager &SensorManager::getInstance(void)
{
    static SensorManager object;
    return object;
}

void SensorManager::addSensor(BaseSensorModule *sensor)
{
    if (sensor == nullptr)
    {
        ESP_LOGE(moduleTag, "didn't added the sensor", );
        return;
    }

    ESP_LOGI(moduleTag, "add sensor %s", sensor->getName().c_str());

    SensorManager &manager = SensorManager::getInstance();
    manager.m_sensors[sensor->getName()] = sensor;
}

BaseSensorModule *SensorManager::getSensor(const std::string &name)
{
    SensorManager &manager = SensorManager::getInstance();
    if (manager.m_sensors.find(name) == manager.m_sensors.end())
        return nullptr;

    return manager.m_sensors[name];
}

void SensorManager::powerOnAll()
{
    SensorManager &manager = SensorManager::getInstance();

    for (auto &it : manager.m_sensors)
        it.second->powerOn();
}

void SensorManager::powerOffAll()
{
    SensorManager &manager = SensorManager::getInstance();

    for (auto &it : manager.m_sensors)
        it.second->powerOff();
}

void SensorManager::resetAll()
{
    SensorManager &manager = SensorManager::getInstance();

    for (auto &it : manager.m_sensors)
        it.second->reset();
}

void SensorManager::readDataAll()
{
    SensorManager &manager = SensorManager::getInstance();

    for (auto &it : manager.m_sensors)
        it.second->readData();
}

void SensorManager::getSensors(std::list<std::string> &list)
{
    list.clear();

    SensorManager &manager = SensorManager::getInstance();

    for (auto &it : manager.m_sensors)
        list.push_back(it.first);
}

void SensorManager::getSensorHeader(const std::string &sensorName, std::map<std::string, uint8_t> &header)
{
    header.clear();

    SensorManager &manager = SensorManager::getInstance();
    if (manager.m_sensors.find(sensorName) == manager.m_sensors.end())
        return;

    manager.m_sensors[sensorName]->getHeader(header);
}

void SensorManager::getSensorData(const std::string &sensorName, const std::string &dataName,
                                  std::vector<time_t> &dataTime, std::vector<uint8_t> &data,
                                  const time_t from, const time_t to)
{
    dataTime.clear();
    data.clear();

    SensorManager &manager = SensorManager::getInstance();
    if (manager.m_sensors.find(sensorName) == manager.m_sensors.end())
        return;

    uint8_t singleDataSize;
    manager.m_sensors[sensorName]->getSingleDataSize(dataName, singleDataSize);
    if (singleDataSize == 0)
        return;

    unsigned int dataCount = MAX_DATA_PACKET_SIZE / singleDataSize;
    manager.m_sensors[sensorName]->getData(dataName, dataTime, data, dataCount, from, to);
}
