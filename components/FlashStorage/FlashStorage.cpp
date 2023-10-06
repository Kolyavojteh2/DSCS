#include "FlashStorage.h"

#include <nvs_flash.h>
#include <esp_system.h>

FlashStorage &FlashStorage::getInstance()
{
    static FlashStorage instance;

    return instance;
}

FlashStorage::FlashStorage()
{
    init();
}

void FlashStorage::init()
{
    esp_err_t ret = nvs_flash_init();     // Flash storage initialization
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) // Error recovery attempt
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}