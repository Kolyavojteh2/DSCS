#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "SensorModules/BaseSensorModule.h"

#include <map>
#include <string>
#include <list>
#include <climits>

#define DEFAULT_READING_PERIOD 1

class SensorManager
{
public:
    static SensorManager &getInstance(void);

    static void addSensor(BaseSensorModule *sensor);
    static BaseSensorModule *getSensor(const std::string &name);

    static void powerOnAll();
    static void powerOffAll();

    static void resetAll();
    static void readDataAll();

    static void getSensors(std::list<std::string> &list);
    static void getSensorHeader(const std::string &sensorName, std::map<std::string, uint8_t> &header);
    static void getSensorData(const std::string &sensorName, const std::string &dataName,
                              std::vector<time_t> &dataTime, std::vector<uint8_t> &data,
                              const time_t from = 0, const time_t to = -1);

    static void getReadingPeriod(time_t &period);
    static void setReadingPeriod(const time_t &period);

private:
    static constexpr const char *moduleTag = "SensorManager";

    SensorManager();
    SensorManager(const SensorManager &) = delete;
    SensorManager &operator=(const SensorManager &) = delete;

    static void readingTask(void *arg);

    std::map<std::string, BaseSensorModule *> m_sensors;
    time_t m_readingPeriod = DEFAULT_READING_PERIOD;
};

#endif // SENSOR_MANAGER_H