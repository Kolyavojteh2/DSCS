#ifndef CCS811_H
#define CCS811_H

#include "../BaseSensorModule.h"
#include <map>

#include <climits>

enum CCS811_ApplicationRegister
{
    CCS811_STATUS = 0x00,
    CCS811_MEAS_MODE = 0x01,
    CCS811_ALG_RESULT_DATA = 0x02,
    CCS811_RAW_DATA = 0x03,
    CCS811_ENV_DATA = 0x05,
    CCS811_NTC = 0x06,
    CCS811_THRESHOLDS = 0x10,
    CCS811_BASELINE = 0x11,
    CCS811_HW_ID = 0x20,
    CCS811_HW_VERSION = 0x21,
    CCS811_FW_BOOT_VERSION = 0x23,
    CCS811_FW_APP_VERSION = 0x24,
    CCS811_ERROR_ID = 0xE0,
    CCS811_SW_RESET = 0xFF,
};

enum CCS811_BootloaderRegister
{
    CCS811_BOOTLOADER_APP_ERASE = 0xF1,
    CCS811_BOOTLOADER_APP_DATA = 0xF2,
    CCS811_BOOTLOADER_APP_VERIFY = 0xF3,
    CCS811_BOOTLOADER_APP_START = 0xF4
};

enum CCS811_StatusCode
{
    CCS811_OK = 0x00,
    CCS811_ERROR = 0x01,
    CCS811_DATA_READY = 0x08,
    CCS811_APP_VALID = 0x10,
    CCS811_FW_MODE_APPLICATION = 0x80,
};

enum CCS811_DriveMode
{
    CCS811_IDLE = 0x00,
    CCS811_CONSTANT_POWER_1S = 0x10,
    CCS811_PULSE_10S = 0x20,
    CCS811_PULSE_60S = 0x30,
    CCS811_CONSTAN_POWER_250MS = 0x40,
};

enum CCS811_ErrorID
{
    CCS811_WRITE_REG_INVALID = 0x01,
    CCS811_READ_REG_INVALID = 0x02,
    CCS811_MEASMODE_INVALID = 0x04,
    CCS811_MAX_RESISTANCE = 0x08,
    CCS811_HEATER_FAULT = 0x10,
    CCS811_HEATER_SUPPLY = 0x20,
};

#define ECO2_STR "eCO2"
#define TVOC_STR "TVOC"

#define CCS811_ADDR 0x5A

class CCS811 : public BaseSensorModule
{
public:
    CCS811();

    void configure() override;
    void reset() override;

    void readData() override;

    void getHeader(std::map<std::string, uint8_t> &header) override;
    void getData(const std::string &dataName,
                 std::vector<time_t> &dataTime, std::vector<uint8_t> &data,
                 unsigned int count,
                 const time_t from = 0, const time_t to = INT_LEAST32_MAX) override;
    void getSingleDataSize(const std::string &dataName, uint8_t &size) override;

    void clearData(const std::string &dataName, const time_t from = 0, const time_t to = INT_LEAST32_MAX);

private:
    void read_ui8(const uint8_t reg, uint8_t &out);
    void read_ui32(const uint8_t reg, uint32_t &out);
    void write_ui8(const uint8_t reg, const uint8_t value);
    void triggerRegister(const uint8_t reg);

    void readStatus();
    void readError();

    bool isDataReady();

    void getDataECO2(std::vector<time_t> &dataTime, std::vector<uint8_t> &data,
                     unsigned int count, const time_t from = 0, const time_t to = INT_LEAST32_MAX);
    void getDataTVOC(std::vector<time_t> &dataTime, std::vector<uint8_t> &data,
                     unsigned int count, const time_t from = 0, const time_t to = INT_LEAST32_MAX);

    std::map<time_t, uint16_t> m_eCO2;
    std::map<time_t, uint16_t> m_TVOC;
};

#endif // CCS811_H