#ifndef HDC1080_H
#define HDC1080_H

#include "../BaseSensorModule.h"

#include <map>

#define HUMIDITY_STR "humidity"
#define TEMPERATURE_STR "temperature"

#define TEMPERATURE_MIN (-40)
#define TEMPERATURE_MAX (125)
#define HUMIDITY_MIN (0)
#define HUMIDITY_MAX (100)

#define HDC1080_ADDR 0x40

enum HDC1080_register
{
    HDC1080_REG_TEMPERATURE = 0x00,
    HDC1080_REG_HUMIDITY = 0x01,
    HDC1080_REG_CONFIG = 0x02
};

class HDC1080 : public BaseSensorModule
{
public:
    HDC1080();

    void readData() override;

    void getHeader(std::map<std::string, uint8_t> &header) override;
    void getData(const std::string &dataName,
                 std::vector<time_t> &dataTime, std::vector<uint8_t> &data,
                 unsigned int count,
                 const time_t from = 0, const time_t to = -1) override;
    void getSingleDataSize(const std::string &dataName, uint8_t &size) override;

private:
    void read_ui16(const uint8_t reg, uint16_t &out);
    void write_ui16(const uint8_t reg, const uint16_t value);
    void triggerMeasure(const uint8_t reg);

    void readTemperature(double &temperature);
    void readHumidity(double &humidity);

    void limitValueToRange(double &value, double range1, double range2);

    void getDataHumidity(std::vector<time_t> &dataTime, std::vector<uint8_t> &data,
                         unsigned int count, const time_t from = 0, const time_t to = -1);
    void getDataTemperature(std::vector<time_t> &dataTime, std::vector<uint8_t> &data,
                            unsigned int count, const time_t from = 0, const time_t to = -1);

    std::map<time_t, uint16_t> m_humidity;
    std::map<time_t, uint16_t> m_temperature;
};

#endif // HDC1080_H