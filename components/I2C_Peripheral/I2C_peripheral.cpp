#include "I2C_peripheral.h"

#include <vector>

#include <esp_err.h>
#include <driver/i2c.h>
#include <esp_log.h>

void I2C_peripheral::init()
{
    static bool isInitialized = false;
    if (isInitialized)
        return;

    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    conf.clk_flags = 0;

    i2c_param_config(I2C_MASTER_NUM, &conf);

    esp_err_t ret = i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
    if (ret == ESP_OK)
        isInitialized = true;
}

void I2C_peripheral::write(const uint8_t deviceAddress,
                           std::vector<uint8_t> &writeData,
                           const TickType_t timeout)
{
    if (writeData.empty())
    {
        ESP_LOGE(moduleTag, "The writeData is empty!");
        return;
    }

    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_write_to_device(I2C_MASTER_NUM, deviceAddress,
                                                             writeData.data(), writeData.size(),
                                                             pdMS_TO_TICKS(timeout)));
}

void I2C_peripheral::read(const uint8_t deviceAddress,
                          std::vector<uint8_t> &readData,
                          const TickType_t timeout)
{
    if (readData.empty())
    {
        ESP_LOGE(moduleTag, "The readData is empty!");
        return;
    }

    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_read_from_device(I2C_MASTER_NUM, deviceAddress,
                                                              readData.data(), readData.size(),
                                                              pdMS_TO_TICKS(timeout)));
}

void I2C_peripheral::writeAndRead(const uint8_t deviceAddress,
                                  std::vector<uint8_t> &writeData,
                                  std::vector<uint8_t> &readData,
                                  const TickType_t timeout)
{
    if (writeData.empty())
    {
        ESP_LOGE(moduleTag, "The writeData is empty!");
        return;
    }

    if (readData.empty())
    {
        ESP_LOGE(moduleTag, "The readData is empty!");
        return;
    }

    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_write_read_device(I2C_MASTER_NUM, deviceAddress,
                                                               writeData.data(), writeData.size(),
                                                               readData.data(), readData.size(),
                                                               pdMS_TO_TICKS(timeout)));
}
