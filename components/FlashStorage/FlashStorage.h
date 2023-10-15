#ifndef FLASH_STORAGE_H
#define FLASH_STORAGE_H

#include <esp_err.h>
#include <nvs_flash.h>

#include <vector>
#include <string>

class FlashStorage
{
public:
    static FlashStorage &getInstance(void);

    static esp_err_t set_i32(const std::string &ns, const std::string &key, const int32_t value);
    static esp_err_t get_i32(const std::string &ns, const std::string &key, int32_t &value);

    static esp_err_t set_u32(const std::string &ns, const std::string &key, const uint32_t value);
    static esp_err_t get_u32(const std::string &ns, const std::string &key, uint32_t &value);

    static esp_err_t set_i64(const std::string &ns, const std::string &key, const int64_t value);
    static esp_err_t get_i64(const std::string &ns, const std::string &key, int64_t &value);

    static esp_err_t set_u64(const std::string &ns, const std::string &key, const uint64_t value);
    static esp_err_t get_u64(const std::string &ns, const std::string &key, uint64_t &value);

    static esp_err_t set_string(const std::string &ns, const std::string &key, const std::string &value);
    static esp_err_t get_string(const std::string &ns, const std::string &key, std::string &value);

    static esp_err_t set_binary(const std::string &ns, const std::string &key, const std::vector<uint8_t> &value);
    static esp_err_t get_binary(const std::string &ns, const std::string &key, std::vector<uint8_t> &value);

private:
    FlashStorage();
    FlashStorage(const FlashStorage &) = delete;
    FlashStorage &operator=(const FlashStorage &) = delete;

    static int checkKeySize(const std::string &key);
    static int checkNamespaceSize(const std::string &ns);

    static nvs_handle_t m_handle;
};

#endif // FLASH_STORAGE_H