#ifndef I2C_PERIPHERAL_H
#define I2C_PERIPHERAL_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <vector>

// TODO: add to the Kconfig
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_SDA_IO (21)
#define I2C_MASTER_SCL_IO (22)
#define I2C_MASTER_FREQ_HZ (100000)
#define I2C_MASTER_TX_BUF_DISABLE (0)
#define I2C_MASTER_RX_BUF_DISABLE (0)

class I2C_peripheral
{
public:
    static constexpr const char *moduleTag = "I2C";

    static void init();

    static void write(const uint8_t deviceAddress,
                      std::vector<uint8_t> &writeData,
                      const TickType_t timeout);

    static void writeAndRead(const uint8_t deviceAddress,
                             std::vector<uint8_t> &writeData,
                             std::vector<uint8_t> &readData,
                             const TickType_t timeout);

    static void read(const uint8_t deviceAddress,
                     std::vector<uint8_t> &readData,
                     const TickType_t timeout);
};

#endif // I2C_PERIPHERAL_H