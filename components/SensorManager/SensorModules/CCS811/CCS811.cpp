#include "CCS811.h"
#include "TimeManager.h"
#include "I2C_peripheral.h"
#include <driver/i2c.h>
#include <esp_log.h>

#ifndef CCS811_I2C_DELAY_FOR_SINGLE_WRITING
#define CCS811_I2C_DELAY_FOR_SINGLE_WRITING 10000
#endif // CCS811_I2C_DELAY_FOR_SINGLE_WRITING

CCS811::CCS811() : BaseSensorModule("CCS811")
{
    I2C_peripheral::init();

    reset();
    vTaskDelay(pdMS_TO_TICKS(100));

    triggerRegister(CCS811_BootloaderRegister::CCS811_BOOTLOADER_APP_START);
    vTaskDelay(pdMS_TO_TICKS(50));

    configure();
    ESP_LOGI(getName().c_str(), "Module was initialized.");
}

void CCS811::configure()
{
    write_ui8(CCS811_ApplicationRegister::CCS811_MEAS_MODE, CCS811_DriveMode::CCS811_CONSTANT_POWER_1S);
    ESP_LOGI(getName().c_str(), "Module was configured.");
}

void CCS811::reset()
{
    std::vector<uint8_t> vecIn = {CCS811_ApplicationRegister::CCS811_SW_RESET, 0x11, 0xE5, 0x72, 0x8A};

    I2C_peripheral::write(CCS811_ADDR, vecIn, CCS811_I2C_DELAY_FOR_SINGLE_WRITING);
    vTaskDelay(pdMS_TO_TICKS(2));
    ESP_LOGI(getName().c_str(), "Module was reset.");
}

void CCS811::readStatus()
{
    uint8_t status;
    read_ui8(CCS811_ApplicationRegister::CCS811_STATUS, status);

    if (status == CCS811_OK)
    {
        ESP_LOGW(getName().c_str(), "Status OK but data not ready");
        return;
    }

    if (status & CCS811_DATA_READY)
        ESP_LOGI(getName().c_str(), "Status Data ready");

    if (status & CCS811_APP_VALID)
        ESP_LOGI(getName().c_str(), "Status App valid");

    if (status & CCS811_FW_MODE_APPLICATION)
        ESP_LOGI(getName().c_str(), "Status mode application");

    if (status & CCS811_ERROR)
    {
        ESP_LOGE(getName().c_str(), "Status error");
        readError();
    }
}

void CCS811::readError()
{
    uint8_t error;
    read_ui8(CCS811_ApplicationRegister::CCS811_ERROR_ID, error);

    if (error & CCS811_WRITE_REG_INVALID)
        ESP_LOGE(getName().c_str(), "CCS811_WRITE_REG_INVALID");
    if (error & CCS811_READ_REG_INVALID)
        ESP_LOGE(getName().c_str(), "CCS811_READ_REG_INVALID");
    if (error & CCS811_MEASMODE_INVALID)
        ESP_LOGE(getName().c_str(), "CCS811_MEASMODE_INVALID");
    if (error & CCS811_MAX_RESISTANCE)
        ESP_LOGE(getName().c_str(), "CCS811_MAX_RESISTANCE");
    if (error & CCS811_HEATER_FAULT)
        ESP_LOGE(getName().c_str(), "CCS811_HEATER_FAULT");
    if (error & CCS811_HEATER_SUPPLY)
        ESP_LOGE(getName().c_str(), "CCS811_HEATER_SUPPLY");
}

void CCS811::readData()
{
    if (!isDataReady())
    {
        ESP_LOGW(getName().c_str(), "Data not ready");
        return;
    }

    uint32_t data;
    read_ui32(CCS811_ApplicationRegister::CCS811_ALG_RESULT_DATA, data);

    uint16_t eCO2 = data >> 16;
    uint16_t TVOC = data & 16;

    time_t currentTime = TimeManager::getTime();
    m_eCO2[currentTime] = eCO2;
    m_TVOC[currentTime] = TVOC;

    ESP_LOGI(getName().c_str(), "Time: %llu, eCO2: %d, TVOC: %d", currentTime, eCO2, TVOC);
}

bool CCS811::isDataReady()
{
    uint8_t status;
    read_ui8(CCS811_ApplicationRegister::CCS811_STATUS, status);

    if (status & CCS811_DATA_READY)
        return true;

    return false;
}

void CCS811::read_ui8(const uint8_t reg, uint8_t &out)
{
    std::vector<uint8_t> vecIn = {reg};
    std::vector<uint8_t> vecOut(sizeof(out));

    I2C_peripheral::writeAndRead(CCS811_ADDR, vecIn, vecOut, CCS811_I2C_DELAY_FOR_SINGLE_WRITING);

    out = vecOut[0];
}

void CCS811::read_ui32(const uint8_t reg, uint32_t &out)
{
    std::vector<uint8_t> vecIn = {reg};
    std::vector<uint8_t> vecOut(sizeof(out));

    I2C_peripheral::writeAndRead(CCS811_ADDR, vecIn, vecOut, CCS811_I2C_DELAY_FOR_SINGLE_WRITING);

    out = ((uint32_t)vecOut[0]) << 24 | ((uint32_t)vecOut[1] << 16) | ((uint32_t)vecOut[2] << 8) | ((uint32_t)vecOut[3]);
}

void CCS811::write_ui8(const uint8_t reg, const uint8_t value)
{
    std::vector<uint8_t> vecIn = {reg, value};

    I2C_peripheral::write(CCS811_ADDR, vecIn, CCS811_I2C_DELAY_FOR_SINGLE_WRITING);
}

void CCS811::triggerRegister(const uint8_t reg)
{
    std::vector<uint8_t> vecIn = {reg};

    I2C_peripheral::write(CCS811_ADDR, vecIn, CCS811_I2C_DELAY_FOR_SINGLE_WRITING);
}

void CCS811::getHeader(std::map<std::string, uint8_t> &header)
{
    header[ECO2_STR] = sizeof(uint16_t);
    header[TVOC_STR] = sizeof(uint16_t);
}

void CCS811::getData(const std::string &dataName,
                     std::vector<time_t> &dataTime, std::vector<uint8_t> &data,
                     unsigned int count,
                     const time_t from, const time_t to)
{
    if (dataName == ECO2_STR)
        getDataECO2(dataTime, data, count, from, to);
    else if (dataName == TVOC_STR)
        getDataTVOC(dataTime, data, count, from, to);
}

void CCS811::getSingleDataSize(const std::string &dataName, uint8_t &size)
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

void CCS811::getDataECO2(std::vector<time_t> &dataTime, std::vector<uint8_t> &data,
                         unsigned int count, const time_t from, const time_t to)
{
    for (auto &it : m_eCO2)
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

void CCS811::getDataTVOC(std::vector<time_t> &dataTime, std::vector<uint8_t> &data,
                         unsigned int count, const time_t from, const time_t to)
{
    for (auto &it : m_TVOC)
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

void CCS811::clearData(const std::string &dataName, const time_t from, const time_t to)
{
    std::map<time_t, uint16_t> *ptrData = nullptr;

    if (dataName == ECO2_STR)
        ptrData = &m_eCO2;
    else if (dataName == TVOC_STR)
        ptrData = &m_TVOC;

    if (ptrData == nullptr)
        return;

    for (auto it = ptrData->begin(); it != ptrData->end();)
    {
        if (it->first >= from && it->first <= to)
            it = ptrData->erase(it);
        else
            ++it;
    }
}
