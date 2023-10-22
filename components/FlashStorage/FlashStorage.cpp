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
}

int FlashStorage::checkNamespaceSize(const std::string &ns)
{
    // TODO: remove hardcode
    if (ns.size() > 15)
        return -1;

    return 0;
}

int FlashStorage::checkKeySize(const std::string &key)
{
    // TODO: remove hardcode
    if (key.size() > 15)
        return -1;

    return 0;
}

esp_err_t FlashStorage::set_i32(const std::string &ns, const std::string &key, const int32_t value)
{
    if (checkKeySize(key))
        return ESP_FAIL;

    if (checkNamespaceSize(ns))
        return ESP_FAIL;

    nvs_handle_t handle;
    esp_err_t ret = nvs_open(ns.c_str(), NVS_READWRITE, &handle);
    if (ret != ESP_OK)
        return ret;

    ret = nvs_set_i32(handle, key.c_str(), value);
    if (ret != ESP_OK)
    {
        nvs_close(handle);
        return ret;
    }

    ret = nvs_commit(handle);
    if (ret != ESP_OK)
    {
        nvs_close(handle);
        return ret;
    }

    nvs_close(handle);
    return ESP_OK;
}

esp_err_t FlashStorage::get_i32(const std::string &ns, const std::string &key, int32_t &value)
{
    if (checkKeySize(key))
        return ESP_FAIL;

    if (checkNamespaceSize(ns))
        return ESP_FAIL;

    nvs_handle_t handle;
    esp_err_t ret = nvs_open(ns.c_str(), NVS_READWRITE, &handle);
    if (ret != ESP_OK)
        return ret;

    ret = nvs_get_i32(handle, key.c_str(), &value);
    if (ret != ESP_OK)
    {
        nvs_close(handle);
        return ret;
    }

    nvs_close(handle);
    return ESP_OK;
}

esp_err_t FlashStorage::set_u32(const std::string &ns, const std::string &key, const uint32_t value)
{
    if (checkKeySize(key))
        return ESP_FAIL;

    if (checkNamespaceSize(ns))
        return ESP_FAIL;

    nvs_handle_t handle;
    esp_err_t ret = nvs_open(ns.c_str(), NVS_READWRITE, &handle);
    if (ret != ESP_OK)
        return ret;

    ret = nvs_set_u32(handle, key.c_str(), value);
    if (ret != ESP_OK)
    {
        nvs_close(handle);
        return ret;
    }

    ret = nvs_commit(handle);
    if (ret != ESP_OK)
    {
        nvs_close(handle);
        return ret;
    }

    nvs_close(handle);
    return ESP_OK;
}

esp_err_t FlashStorage::get_u32(const std::string &ns, const std::string &key, uint32_t &value)
{
    if (checkKeySize(key))
        return ESP_FAIL;

    if (checkNamespaceSize(ns))
        return ESP_FAIL;

    nvs_handle_t handle;
    esp_err_t ret = nvs_open(ns.c_str(), NVS_READWRITE, &handle);
    if (ret != ESP_OK)
        return ret;

    // TODO: change nvs_get_u32
    ret = nvs_get_u32(handle, key.c_str(), &value);
    if (ret != ESP_OK)
    {
        nvs_close(handle);
        return ret;
    }

    nvs_close(handle);
    return ESP_OK;
}

esp_err_t FlashStorage::set_i64(const std::string &ns, const std::string &key, const int64_t value)
{
    if (checkKeySize(key))
        return ESP_FAIL;

    if (checkNamespaceSize(ns))
        return ESP_FAIL;

    nvs_handle_t handle;
    esp_err_t ret = nvs_open(ns.c_str(), NVS_READWRITE, &handle);
    if (ret != ESP_OK)
        return ret;

    ret = nvs_set_i64(handle, key.c_str(), value);
    if (ret != ESP_OK)
    {
        nvs_close(handle);
        return ret;
    }

    ret = nvs_commit(handle);
    if (ret != ESP_OK)
    {
        nvs_close(handle);
        return ret;
    }

    nvs_close(handle);
    return ESP_OK;
}

esp_err_t FlashStorage::get_i64(const std::string &ns, const std::string &key, int64_t &value)
{
    if (checkKeySize(key))
        return ESP_FAIL;

    if (checkNamespaceSize(ns))
        return ESP_FAIL;

    nvs_handle_t handle;
    esp_err_t ret = nvs_open(ns.c_str(), NVS_READWRITE, &handle);
    if (ret != ESP_OK)
        return ret;

    // TODO: change nvs_get_u32
    ret = nvs_get_i64(handle, key.c_str(), &value);
    if (ret != ESP_OK)
    {
        nvs_close(handle);
        return ret;
    }

    nvs_close(handle);
    return ESP_OK;
}

esp_err_t FlashStorage::set_u64(const std::string &ns, const std::string &key, const uint64_t value)
{
    if (checkKeySize(key))
        return ESP_FAIL;

    if (checkNamespaceSize(ns))
        return ESP_FAIL;

    nvs_handle_t handle;
    esp_err_t ret = nvs_open(ns.c_str(), NVS_READWRITE, &handle);
    if (ret != ESP_OK)
        return ret;

    ret = nvs_set_u64(handle, key.c_str(), value);
    if (ret != ESP_OK)
    {
        nvs_close(handle);
        return ret;
    }

    ret = nvs_commit(handle);
    if (ret != ESP_OK)
    {
        nvs_close(handle);
        return ret;
    }

    nvs_close(handle);
    return ESP_OK;
}

esp_err_t FlashStorage::get_u64(const std::string &ns, const std::string &key, uint64_t &value)
{
    if (checkKeySize(key))
        return ESP_FAIL;

    if (checkNamespaceSize(ns))
        return ESP_FAIL;

    nvs_handle_t handle;
    esp_err_t ret = nvs_open(ns.c_str(), NVS_READWRITE, &handle);
    if (ret != ESP_OK)
        return ret;

    // TODO: change nvs_get_u32
    ret = nvs_get_u64(handle, key.c_str(), &value);
    if (ret != ESP_OK)
    {
        nvs_close(handle);
        return ret;
    }

    nvs_close(handle);
    return ESP_OK;
}

esp_err_t FlashStorage::set_string(const std::string &ns, const std::string &key, const std::string &value)
{
    if (checkKeySize(key))
        return ESP_FAIL;

    if (checkNamespaceSize(ns))
        return ESP_FAIL;

    nvs_handle_t handle;
    esp_err_t ret = nvs_open(ns.c_str(), NVS_READWRITE, &handle);
    if (ret != ESP_OK)
        return ret;

    ret = nvs_set_str(handle, key.c_str(), value.c_str());
    if (ret != ESP_OK)
    {
        nvs_close(handle);
        return ret;
    }

    ret = nvs_commit(handle);
    if (ret != ESP_OK)
    {
        nvs_close(handle);
        return ret;
    }

    nvs_close(handle);
    return ESP_OK;
}

esp_err_t FlashStorage::get_string(const std::string &ns, const std::string &key, std::string &value)
{
    if (checkKeySize(key))
        return ESP_FAIL;

    if (checkNamespaceSize(ns))
        return ESP_FAIL;

    nvs_handle_t handle;
    esp_err_t ret = nvs_open(ns.c_str(), NVS_READWRITE, &handle);
    if (ret != ESP_OK)
        return ret;

    // get string size
    size_t strSize = 0;
    ret = nvs_get_str(handle, key.c_str(), NULL, &strSize);
    if (ret != ESP_OK)
    {
        nvs_close(handle);
        return ret;
    }

    if (strSize == 0)
    {
        nvs_close(handle);
        return ESP_FAIL;
    }

    // read string data
    char *buffer = new char[strSize];
    ret = nvs_get_str(handle, key.c_str(), buffer, &strSize);
    if (ret != ESP_OK)
    {
        delete[] buffer;
        nvs_close(handle);
        return ret;
    }

    value.assign(buffer);

    nvs_close(handle);
    return ESP_OK;
}

esp_err_t FlashStorage::set_binary(const std::string &ns, const std::string &key, const std::vector<uint8_t> &value)
{
    if (checkKeySize(key))
        return ESP_FAIL;

    if (checkNamespaceSize(ns))
        return ESP_FAIL;

    nvs_handle_t handle;
    esp_err_t ret = nvs_open(ns.c_str(), NVS_READWRITE, &handle);
    if (ret != ESP_OK)
        return ret;

    ret = nvs_set_blob(handle, key.c_str(), (const void *)value.data(), value.size());
    if (ret != ESP_OK)
    {
        nvs_close(handle);
        return ret;
    }

    ret = nvs_commit(handle);
    if (ret != ESP_OK)
    {
        nvs_close(handle);
        return ret;
    }

    nvs_close(handle);
    return ESP_OK;
}

esp_err_t FlashStorage::get_binary(const std::string &ns, const std::string &key, std::vector<uint8_t> &value)
{
    if (checkKeySize(key))
        return ESP_FAIL;

    if (checkNamespaceSize(ns))
        return ESP_FAIL;

    nvs_handle_t handle;
    esp_err_t ret = nvs_open(ns.c_str(), NVS_READWRITE, &handle);
    if (ret != ESP_OK)
        return ret;

    // get string size
    size_t binSize = 0;
    ret = nvs_get_blob(handle, key.c_str(), NULL, &binSize);
    if (ret != ESP_OK)
    {
        nvs_close(handle);
        return ret;
    }

    // read string data
    uint8_t *buffer = new uint8_t[binSize];
    ret = nvs_get_blob(handle, key.c_str(), buffer, &binSize);
    if (ret != ESP_OK)
    {
        delete[] buffer;
        nvs_close(handle);
        return ret;
    }

    value = std::move(std::vector<uint8_t>(buffer, buffer + binSize));

    nvs_close(handle);
    return ESP_OK;
}
