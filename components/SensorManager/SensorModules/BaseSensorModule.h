#ifndef BASE_SENSOR_MODULE_H
#define BASE_SENSOR_MODULE_H

#include <string>
#include <vector>
#include <map>

class BaseSensorModule
{
public:
    BaseSensorModule(const std::string &name);

    const std::string &getName() const;

    virtual void configure();
    virtual void reset();

    virtual void powerOn();
    virtual void powerOff();

    virtual void readData() = 0;

    virtual void getHeader(std::map<std::string, uint8_t> &header) = 0;
    virtual void getData(const std::string &dataName,
                         std::vector<time_t> &dataTime, std::vector<uint8_t> &data,
                         unsigned int count,
                         const time_t from = 0, const time_t to = -1) = 0;
    virtual void getSingleDataSize(const std::string &dataName, uint8_t &size) = 0;

    virtual void clearData(const std::string &dataName, const time_t from = 0, const time_t to = -1);

protected:
    std::string m_name;
};

#endif // BASE_SENSOR_MODULE_H