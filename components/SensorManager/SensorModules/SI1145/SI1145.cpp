#include "SI1145.h"
#include "I2C_peripheral.h"
#include "TimeManager.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

#define I2C_DELAY_FOR_SINGLE_WRITING 1000

SI1145::SI1145() : BaseSensorModule("SI1145")
{
    I2C_peripheral::init();

    uint8_t id;
    read_ui8(SI1145_REG_PARTID, id);
    if (id != 0x45)
        return; // look for SI1145

    reset();

    // enable UVindex measurement coefficients
    write_ui8(SI1145_REG_UCOEFF0, 0x29);
    write_ui8(SI1145_REG_UCOEFF1, 0x89);
    write_ui8(SI1145_REG_UCOEFF2, 0x02);
    write_ui8(SI1145_REG_UCOEFF3, 0x00);

    // enable UV sensor
    write_parameter(SI1145_PARAM_CHLIST, SI1145_PARAM_CHLIST_ENUV | SI1145_PARAM_CHLIST_ENALSIR |
                                             SI1145_PARAM_CHLIST_ENALSVIS | SI1145_PARAM_CHLIST_ENPS1);

    // enable interrupt on every sample
    write_ui8(SI1145_REG_INTCFG, SI1145_REG_INTCFG_INTOE);
    write_ui8(SI1145_REG_IRQEN, SI1145_REG_IRQEN_ALSEVERYSAMPLE);

    // Prox Sense 1
    // program LED current
    write_ui8(SI1145_REG_PSLED21, 0x03); // 20mA for LED 1 only
    write_parameter(SI1145_PARAM_PS1ADCMUX, SI1145_PARAM_ADCMUX_LARGEIR);
    // prox sensor #1 uses LED #1
    write_parameter(SI1145_PARAM_PSLED12SEL, SI1145_PARAM_PSLED12SEL_PS1LED1);
    // fastest clocks, clock div 1
    write_parameter(SI1145_PARAM_PSADCGAIN, 0);
    // take 511 clocks to measure
    write_parameter(SI1145_PARAM_PSADCOUNTER, SI1145_PARAM_ADCCOUNTER_511CLK);
    // in prox mode, high range
    write_parameter(SI1145_PARAM_PSADCMISC,
                    SI1145_PARAM_PSADCMISC_RANGE | SI1145_PARAM_PSADCMISC_PSMODE);

    write_parameter(SI1145_PARAM_ALSIRADCMUX, SI1145_PARAM_ADCMUX_SMALLIR);
    // fastest clocks, clock div 1
    write_parameter(SI1145_PARAM_ALSIRADCGAIN, 0);
    // take 511 clocks to measure
    write_parameter(SI1145_PARAM_ALSIRADCOUNTER, SI1145_PARAM_ADCCOUNTER_511CLK);
    // in high range mode
    write_parameter(SI1145_PARAM_ALSIRADCMISC, SI1145_PARAM_ALSIRADCMISC_RANGE);

    // fastest clocks, clock div 1
    write_parameter(SI1145_PARAM_ALSVISADCGAIN, 0);
    // take 511 clocks to measure
    write_parameter(SI1145_PARAM_ALSVISADCOUNTER, SI1145_PARAM_ADCCOUNTER_511CLK);
    // in high range mode (not normal signal)
    write_parameter(SI1145_PARAM_ALSVISADCMISC, SI1145_PARAM_ALSVISADCMISC_VISRANGE);

    // measurement rate for auto
    write_ui8(SI1145_REG_MEASRATE0, 0xFF); // 255 * 31.25uS = 8ms
    // auto run
    write_ui8(SI1145_REG_COMMAND, SI1145_PSALS_AUTO);
}

void SI1145::reset()
{
    write_ui8(SI1145_REG_MEASRATE0, 0);
    write_ui8(SI1145_REG_MEASRATE1, 0);
    write_ui8(SI1145_REG_IRQEN, 0);
    write_ui8(SI1145_REG_IRQMODE1, 0);
    write_ui8(SI1145_REG_IRQMODE2, 0);
    write_ui8(SI1145_REG_INTCFG, 0);
    write_ui8(SI1145_REG_IRQSTAT, 0xFF);
    write_ui8(SI1145_REG_COMMAND, SI1145_RESET);
    vTaskDelay(pdMS_TO_TICKS(10));

    write_ui8(SI1145_REG_HWKEY, 0x17);
    vTaskDelay(pdMS_TO_TICKS(10));
}

void SI1145::read_ui8(const uint8_t reg, uint8_t &out)
{
    std::vector<uint8_t> vecIn = {reg};
    std::vector<uint8_t> vecOut(sizeof(uint8_t));

    I2C_peripheral::writeAndRead(SI1145_ADDR, vecIn, vecOut, I2C_DELAY_FOR_SINGLE_WRITING);

    out = vecOut[0];
}

void SI1145::read_ui16(const uint8_t reg, uint16_t &out)
{
    std::vector<uint8_t> vecIn = {reg};
    std::vector<uint8_t> vecOut(sizeof(uint16_t));

    I2C_peripheral::writeAndRead(SI1145_ADDR, vecIn, vecOut, I2C_DELAY_FOR_SINGLE_WRITING);

    out = ((uint16_t)vecOut[0]) | ((uint16_t)vecOut[1] << 8);
}

void SI1145::write_ui8(const uint8_t reg, const uint8_t value)
{
    std::vector<uint8_t> vecIn = {reg, value};

    I2C_peripheral::write(SI1145_ADDR, vecIn, I2C_DELAY_FOR_SINGLE_WRITING);
}

uint8_t SI1145::write_parameter(uint8_t parameter, uint8_t value)
{
    write_ui8(SI1145_REG_PARAMWR, value);
    write_ui8(SI1145_REG_COMMAND, parameter | SI1145_PARAM_SET);

    uint8_t result;
    read_ui8(SI1145_REG_PARAMRD, result);

    return result;
}

void SI1145::readUV(uint16_t &out)
{
    read_ui16(SI1145_REG_UVINDEX0, out);
}

void SI1145::readVisible(uint16_t &out)
{
    read_ui16(SI1145_REG_ALSVISDATA0, out);
}

void SI1145::readIR(uint16_t &out)
{
    read_ui16(SI1145_REG_ALSIRDATA0, out);
}

void SI1145::readProx(uint16_t &out)
{
    read_ui16(SI1145_REG_PS1DATA0, out);
}

void SI1145::readData()
{
    uint16_t uv;
    readUV(uv);

    uint16_t visible;
    readVisible(visible);

    uint16_t ir;
    readIR(ir);

    uint16_t prox;
    readProx(prox);

    time_t currentTime = TimeManager::getTime();

    m_UV[currentTime] = uv;
    m_Visible[currentTime] = visible;
    m_IR[currentTime] = ir;
    m_Prox[currentTime] = prox;

    ESP_LOGI(getName().c_str(), "Time: %llu, UV: %d, Visible: %d, IR: %d, Prox: %d",
             currentTime, uv, visible, ir, prox);
}

void SI1145::getHeader(std::map<std::string, uint8_t> &header)
{
    header[UV_STR] = sizeof(uint16_t);
    header[VISIBLE_STR] = sizeof(uint16_t);
    header[IR_STR] = sizeof(uint16_t);
    header[PROX_STR] = sizeof(uint16_t);
}

void SI1145::getData(const std::string &dataName,
                     std::vector<time_t> &dataTime, std::vector<uint8_t> &data,
                     unsigned int count,
                     const time_t from, const time_t to)
{
    if (dataName == UV_STR)
        getUVData(dataTime, data, count, from, to);
    else if (dataName == VISIBLE_STR)
        getVisibleData(dataTime, data, count, from, to);
    else if (dataName == IR_STR)
        getIRData(dataTime, data, count, from, to);
    else if (dataName == PROX_STR)
        getProxData(dataTime, data, count, from, to);
}

void SI1145::getSingleDataSize(const std::string &dataName, uint8_t &size)
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

void SI1145::getUVData(std::vector<time_t> &dataTime, std::vector<uint8_t> &data,
                       unsigned int count,
                       const time_t from, const time_t to)
{
    for (auto &it : m_UV)
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

void SI1145::getVisibleData(std::vector<time_t> &dataTime, std::vector<uint8_t> &data,
                            unsigned int count,
                            const time_t from, const time_t to)
{
    for (auto &it : m_Visible)
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

void SI1145::getIRData(std::vector<time_t> &dataTime, std::vector<uint8_t> &data,
                       unsigned int count,
                       const time_t from, const time_t to)
{
    for (auto &it : m_IR)
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

void SI1145::getProxData(std::vector<time_t> &dataTime, std::vector<uint8_t> &data,
                         unsigned int count,
                         const time_t from, const time_t to)
{
    for (auto &it : m_Prox)
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

void SI1145::clearData(const std::string &dataName, const time_t from, const time_t to)
{
    std::map<time_t, uint16_t> *ptrData = nullptr;

    if (dataName == UV_STR)
        ptrData = &m_UV;
    else if (dataName == VISIBLE_STR)
        ptrData = &m_Visible;
    else if (dataName == IR_STR)
        ptrData = &m_IR;
    else if (dataName == PROX_STR)
        ptrData = &m_Prox;

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