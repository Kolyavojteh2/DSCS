#include "FlashStorage.h"

#include <esp_system.h>

nvs_handle_t FlashStorage::m_handle = 0;

FlashStorage &FlashStorage::getInstance()
{
    static FlashStorage instance;

    return instance;
}

FlashStorage::FlashStorage()
{
    // Init flash storage
    esp_err_t ret = nvs_flash_init();     // Flash storage initialization
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) // Error recovery attempt
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // TODO: remove hardcode
    ret = nvs_open("storage", NVS_READWRITE, &m_handle);

    if (ret != ESP_OK)
        return;
}

esp_err_t FlashStorage::set_i32(const std::string &key, const int32_t value)
{
    esp_err_t ret = nvs_set_i32(m_handle, key.c_str(), value);
    if (ret != ESP_OK)
        return ret;

    return nvs_commit(m_handle);
}

esp_err_t FlashStorage::get_i32(const std::string &key, int32_t &value)
{
    return nvs_get_i32(m_handle, key.c_str(), &value);
}

esp_err_t FlashStorage::set_u32(const std::string &key, const uint32_t value)
{
    esp_err_t ret = nvs_set_u32(m_handle, key.c_str(), value);
    if (ret != ESP_OK)
        return ret;

    return nvs_commit(m_handle);
}

esp_err_t FlashStorage::get_u32(const std::string &key, uint32_t &value)
{
    return nvs_get_u32(m_handle, key.c_str(), &value);
}

esp_err_t FlashStorage::set_i64(const std::string &key, const int64_t value)
{
    esp_err_t ret = nvs_set_i64(m_handle, key.c_str(), value);
    if (ret != ESP_OK)
        return ret;

    return nvs_commit(m_handle);
}

esp_err_t FlashStorage::get_i64(const std::string &key, int64_t &value)
{
    return nvs_get_i64(m_handle, key.c_str(), &value);
}

esp_err_t FlashStorage::set_u64(const std::string &key, const uint64_t value)
{
    esp_err_t ret = nvs_set_u64(m_handle, key.c_str(), value);
    if (ret != ESP_OK)
        return ret;

    return nvs_commit(m_handle);
}

esp_err_t FlashStorage::get_u64(const std::string &key, uint64_t &value)
{
    return nvs_get_u64(m_handle, key.c_str(), &value);
}

esp_err_t FlashStorage::set_string(const std::string &key, const std::string &value)
{
    esp_err_t ret = nvs_set_str(m_handle, key.c_str(), value.c_str());
    if (ret != ESP_OK)
        return ret;

    return nvs_commit(m_handle);
}

esp_err_t FlashStorage::get_string(const std::string &key, std::string &value)
{
    // get string size
    size_t strSize = 0;
    esp_err_t ret = nvs_get_str(m_handle, key.c_str(), NULL, &strSize);
    if (ret != ESP_OK)
        return ret;

    // read string data
    char *buffer = new char[strSize];
    ret = nvs_get_str(m_handle, key.c_str(), buffer, &strSize);
    if (ret != ESP_OK)
    {
        delete[] buffer;
        return ret;
    }

    value.assign(buffer);

    delete[] buffer;
    return ret;
}

esp_err_t FlashStorage::set_binary(const std::string &key, const std::vector<uint8_t> &value)
{
    esp_err_t ret = nvs_set_blob(m_handle, key.c_str(), (const void *)value.data(), value.size());
    if (ret != ESP_OK)
        return ret;

    return nvs_commit(m_handle);
}

esp_err_t FlashStorage::get_binary(const std::string &key, std::vector<uint8_t> &value)
{
    // get binary size
    size_t binSize = 0;
    esp_err_t ret = nvs_get_blob(m_handle, key.c_str(), NULL, &binSize);
    if (ret != ESP_OK)
        return ret;

    // read binary data
    uint8_t *buffer = new uint8_t[binSize];
    ret = nvs_get_blob(m_handle, key.c_str(), buffer, &binSize);
    if (ret != ESP_OK)
    {
        delete[] buffer;
        return ret;
    }
    value = std::move(std::vector<uint8_t>(buffer, buffer + binSize));

    delete[] buffer;
    return ret;
}
