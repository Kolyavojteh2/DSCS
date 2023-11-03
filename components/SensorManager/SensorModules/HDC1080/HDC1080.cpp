#include "HDC1080.h"
#include "TimeManager.h"
#include "I2C_peripheral.h"
#include <cmath>

#include <esp_log.h>

#define HDC1080_I2C_DELAY_FOR_SINGLE_WRITING 1000

// TODO: change the values according to the datasheet
static constexpr const double HUMIDITY_MULTIPLIER = 0.0015258789;
static constexpr const double TEMPERATURE_MULTIPLIER = 0.00251770019;
static constexpr const int TEMPERATURE_OFFSET = 40;

HDC1080::HDC1080() : BaseSensorModule("HDC1080")
{
    I2C_peripheral::init();

    // TODO: humidity and temperature resolution hardcode
    // TODO: add additional enums
    write_ui16(HDC1080_register::HDC1080_REG_CONFIG, 0x10);
}

void HDC1080::readData()
{
    double temperature;
    readTemperature(temperature);

    double humidity;
    readHumidity(humidity);

    time_t currentTime = TimeManager::getTime();
    m_temperature[currentTime] = temperature;
    m_humidity[currentTime] = humidity;

    ESP_LOGI(getName().c_str(), "Time: %llu, humidity: %f, temperature: %f",
             currentTime, humidity, temperature);
}

void HDC1080::readTemperature(double &temperature)
{
    // Start measuring and wait until it will be done
    triggerMeasure(HDC1080_register::HDC1080_REG_TEMPERATURE);
    vTaskDelay(pdMS_TO_TICKS(10));

    uint16_t raw;
    read_ui16(HDC1080_register::HDC1080_REG_TEMPERATURE, raw);

    temperature = (((double)raw) / pow(2, 16)) * (TEMPERATURE_MAX - TEMPERATURE_MIN) + TEMPERATURE_MIN;
    limitValueToRange(temperature, TEMPERATURE_MIN, TEMPERATURE_MAX);
}

void HDC1080::readHumidity(double &humidity)
{
    // Start measuring and wait until it will be done
    triggerMeasure(HDC1080_register::HDC1080_REG_HUMIDITY);
    vTaskDelay(pdMS_TO_TICKS(10));

    uint16_t raw;
    read_ui16(HDC1080_register::HDC1080_REG_HUMIDITY, raw);

    humidity = (double)raw * HUMIDITY_MULTIPLIER;
    limitValueToRange(humidity, HUMIDITY_MIN, HUMIDITY_MAX);
}

void HDC1080::triggerMeasure(const uint8_t reg)
{
    std::vector<uint8_t> vecIn = {reg};
    I2C_peripheral::write(HDC1080_ADDR, vecIn, HDC1080_I2C_DELAY_FOR_SINGLE_WRITING);
}

void HDC1080::read_ui16(const uint8_t reg, uint16_t &out)
{
    std::vector<uint8_t> vecIn = {reg};
    std::vector<uint8_t> vecOut(sizeof(uint16_t));

    I2C_peripheral::write(HDC1080_ADDR, vecIn, HDC1080_I2C_DELAY_FOR_SINGLE_WRITING);
    vTaskDelay(pdMS_TO_TICKS(15));
    I2C_peripheral::read(HDC1080_ADDR, vecOut, HDC1080_I2C_DELAY_FOR_SINGLE_WRITING);

    // TODO: check the delay 15 ms

    out = ((uint16_t)vecOut[0] << 8) | ((uint16_t)vecOut[1]);
}

void HDC1080::write_ui16(const uint8_t reg, const uint16_t value)
{
    uint8_t MSB_value = value >> 8;
    uint8_t LSB_value = value & 0xFF;

    std::vector<uint8_t> vecIn = {reg, MSB_value, LSB_value};

    I2C_peripheral::write(HDC1080_ADDR, vecIn, HDC1080_I2C_DELAY_FOR_SINGLE_WRITING);
    vTaskDelay(pdMS_TO_TICKS(15));
    // TODO: check the delay 15 ms
}

void HDC1080::limitValueToRange(double &value, double range1, double range2)
{
    if (range1 > range2)
        std::swap(range1, range2);

    if (value < range1)
        value = range1;
    else if (value > range2)
        value = range2;
}

void HDC1080::getHeader(std::map<std::string, uint8_t> &header)
{
    header[HUMIDITY_STR] = sizeof(uint16_t);
    header[TEMPERATURE_STR] = sizeof(uint16_t);
}

void HDC1080::getData(const std::string &dataName,
                      std::vector<time_t> &dataTime, std::vector<uint8_t> &data,
                      unsigned int count,
                      const time_t from, const time_t to)
{
    if (dataName == HUMIDITY_STR)
        getDataHumidity(dataTime, data, count, from, to);
    else if (dataName == TEMPERATURE_STR)
        getDataTemperature(dataTime, data, count, from, to);
}

void HDC1080::getSingleDataSize(const std::string &dataName, uint8_t &size)
{
    std::map<std::string, uint8_t> header;
    getHeader(header);

    if (header.find(dataName) == header.end())
    {
        size = 0;
        return;
    }

    size = sizeof(time_t) + header[dataName];
}

void HDC1080::getDataHumidity(std::vector<time_t> &dataTime, std::vector<uint8_t> &data,
                              unsigned int count, const time_t from, const time_t to)
{
    for (auto &it : m_humidity)
    {
        if (count == 0)
            break;

        if (it.first < from || it.first > to)
            continue;

        dataTime.push_back(it.first);
        data.push_back(it.second & 0xFF);
        data.push_back(it.second >> 8);

        count--;
    }
}

void HDC1080::getDataTemperature(std::vector<time_t> &dataTime, std::vector<uint8_t> &data,
                                 unsigned int count, const time_t from, const time_t to)
{
    for (auto &it : m_temperature)
    {
        if (count == 0)
            break;

        if (it.first < from || it.first > to)
            continue;

        dataTime.push_back(it.first);
        data.push_back(it.second & 0xFF);
        data.push_back(it.second >> 8);

        count--;
    }
}
