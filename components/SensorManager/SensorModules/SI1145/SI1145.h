#ifndef SI1145_H
#define SI1145_H

#include "../BaseSensorModule.h"

#include <map>

enum SI1145_command
{
    SI1145_PARAM_QUERY = 0x80,
    SI1145_PARAM_SET = 0xA0,
    SI1145_NOP = 0x0,
    SI1145_RESET = 0x01,
    SI1145_BUSADDR = 0x02,
    SI1145_PS_FORCE = 0x05,
    SI1145_ALS_FORCE = 0x06,
    SI1145_PSALS_FORCE = 0x07,
    SI1145_PS_PAUSE = 0x09,
    SI1145_ALS_PAUSE = 0x0A,
    SI1145_PSALS_PAUSE = 0xB,
    SI1145_PS_AUTO = 0x0D,
    SI1145_ALS_AUTO = 0x0E,
    SI1145_PSALS_AUTO = 0x0F,
    SI1145_GET_CAL = 0x12,
};

enum SI1145_parameter
{
    SI1145_PARAM_I2CADDR = 0x00,
    SI1145_PARAM_CHLIST = 0x01,
    SI1145_PARAM_CHLIST_ENUV = 0x80,
    SI1145_PARAM_CHLIST_ENAUX = 0x40,
    SI1145_PARAM_CHLIST_ENALSIR = 0x20,
    SI1145_PARAM_CHLIST_ENALSVIS = 0x10,
    SI1145_PARAM_CHLIST_ENPS1 = 0x01,
    SI1145_PARAM_CHLIST_ENPS2 = 0x02,
    SI1145_PARAM_CHLIST_ENPS3 = 0x04,

    SI1145_PARAM_PSLED12SEL = 0x02,
    SI1145_PARAM_PSLED12SEL_PS2NONE = 0x00,
    SI1145_PARAM_PSLED12SEL_PS2LED1 = 0x10,
    SI1145_PARAM_PSLED12SEL_PS2LED2 = 0x20,
    SI1145_PARAM_PSLED12SEL_PS2LED3 = 0x40,
    SI1145_PARAM_PSLED12SEL_PS1NONE = 0x00,
    SI1145_PARAM_PSLED12SEL_PS1LED1 = 0x01,
    SI1145_PARAM_PSLED12SEL_PS1LED2 = 0x02,
    SI1145_PARAM_PSLED12SEL_PS1LED3 = 0x04,

    SI1145_PARAM_PSLED3SEL = 0x03,
    SI1145_PARAM_PSENCODE = 0x05,
    SI1145_PARAM_ALSENCODE = 0x06,

    SI1145_PARAM_PS1ADCMUX = 0x07,
    SI1145_PARAM_PS2ADCMUX = 0x08,
    SI1145_PARAM_PS3ADCMUX = 0x09,
    SI1145_PARAM_PSADCOUNTER = 0x0A,
    SI1145_PARAM_PSADCGAIN = 0x0B,
    SI1145_PARAM_PSADCMISC = 0x0C,
    SI1145_PARAM_PSADCMISC_RANGE = 0x20,
    SI1145_PARAM_PSADCMISC_PSMODE = 0x04,

    SI1145_PARAM_ALSIRADCMUX = 0x0E,
    SI1145_PARAM_AUXADCMUX = 0x0F,

    SI1145_PARAM_ALSVISADCOUNTER = 0x10,
    SI1145_PARAM_ALSVISADCGAIN = 0x11,
    SI1145_PARAM_ALSVISADCMISC = 0x12,
    SI1145_PARAM_ALSVISADCMISC_VISRANGE = 0x20,

    SI1145_PARAM_ALSIRADCOUNTER = 0x1D,
    SI1145_PARAM_ALSIRADCGAIN = 0x1E,
    SI1145_PARAM_ALSIRADCMISC = 0x1F,
    SI1145_PARAM_ALSIRADCMISC_RANGE = 0x20,

    SI1145_PARAM_ADCCOUNTER_511CLK = 0x70,

    SI1145_PARAM_ADCMUX_SMALLIR = 0x00,
    SI1145_PARAM_ADCMUX_LARGEIR = 0x03,
};

enum SI1145_register
{
    SI1145_REG_PARTID = 0x00,
    SI1145_REG_REVID = 0x01,
    SI1145_REG_SEQID = 0x02,

    SI1145_REG_INTCFG = 0x03,
    SI1145_REG_INTCFG_INTOE = 0x01,
    SI1145_REG_INTCFG_INTMODE = 0x02,

    SI1145_REG_IRQEN = 0x04,
    SI1145_REG_IRQEN_ALSEVERYSAMPLE = 0x01,
    SI1145_REG_IRQEN_PS1EVERYSAMPLE = 0x04,
    SI1145_REG_IRQEN_PS2EVERYSAMPLE = 0x08,
    SI1145_REG_IRQEN_PS3EVERYSAMPLE = 0x10,

    SI1145_REG_IRQMODE1 = 0x05,
    SI1145_REG_IRQMODE2 = 0x06,

    SI1145_REG_HWKEY = 0x07,
    SI1145_REG_MEASRATE0 = 0x08,
    SI1145_REG_MEASRATE1 = 0x09,
    SI1145_REG_PSRATE = 0x0A,
    SI1145_REG_PSLED21 = 0x0F,
    SI1145_REG_PSLED3 = 0x10,
    SI1145_REG_UCOEFF0 = 0x13,
    SI1145_REG_UCOEFF1 = 0x14,
    SI1145_REG_UCOEFF2 = 0x15,
    SI1145_REG_UCOEFF3 = 0x16,
    SI1145_REG_PARAMWR = 0x17,
    SI1145_REG_COMMAND = 0x18,
    SI1145_REG_RESPONSE = 0x20,
    SI1145_REG_IRQSTAT = 0x21,
    SI1145_REG_IRQSTAT_ALS = 0x01,

    SI1145_REG_ALSVISDATA0 = 0x22,
    SI1145_REG_ALSVISDATA1 = 0x23,
    SI1145_REG_ALSIRDATA0 = 0x24,
    SI1145_REG_ALSIRDATA1 = 0x25,
    SI1145_REG_PS1DATA0 = 0x26,
    SI1145_REG_PS1DATA1 = 0x27,
    SI1145_REG_PS2DATA0 = 0x28,
    SI1145_REG_PS2DATA1 = 0x29,
    SI1145_REG_PS3DATA0 = 0x2A,
    SI1145_REG_PS3DATA1 = 0x2B,
    SI1145_REG_UVINDEX0 = 0x2C,
    SI1145_REG_UVINDEX1 = 0x2D,
    SI1145_REG_PARAMRD = 0x2E,
    SI1145_REG_CHIPSTAT = 0x30,
};

#define UV_STR "uv"
#define VISIBLE_STR "visible"
#define IR_STR "ir"
#define PROX_STR "prox"

#define SI1145_ADDR 0x60

class SI1145 : public BaseSensorModule
{
public:
    SI1145();

    void reset() override;

    void readData() override;

    void readUV(uint16_t &out);
    void readVisible(uint16_t &out);
    void readIR(uint16_t &out);
    void readProx(uint16_t &out);

    void getHeader(std::map<std::string, uint8_t> &header) override;
    void getData(const std::string &dataName,
                 std::vector<time_t> &dataTime, std::vector<uint8_t> &data,
                 unsigned int count,
                 const time_t from = 0, const time_t to = -1) override;
    void getSingleDataSize(const std::string &dataName, uint8_t &size) override;

private:
    void read_ui8(const uint8_t reg, uint8_t &out);
    void read_ui16(const uint8_t reg, uint16_t &out);
    void write_ui8(const uint8_t reg, const uint8_t value);

    uint8_t write_parameter(uint8_t parameter, uint8_t value);

    void getUVData(std::vector<time_t> &dataTime, std::vector<uint8_t> &data,
                   unsigned int count,
                   const time_t from = 0, const time_t to = -1);
    void getVisibleData(std::vector<time_t> &dataTime, std::vector<uint8_t> &data,
                        unsigned int count,
                        const time_t from = 0, const time_t to = -1);
    void getIRData(std::vector<time_t> &dataTime, std::vector<uint8_t> &data,
                   unsigned int count,
                   const time_t from = 0, const time_t to = -1);
    void getProxData(std::vector<time_t> &dataTime, std::vector<uint8_t> &data,
                     unsigned int count,
                     const time_t from = 0, const time_t to = -1);

    std::map<time_t, uint16_t> m_UV;
    std::map<time_t, uint16_t> m_Visible;
    std::map<time_t, uint16_t> m_IR;
    std::map<time_t, uint16_t> m_Prox;
};

#endif // SI1145_H