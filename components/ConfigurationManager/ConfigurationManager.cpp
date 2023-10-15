#include "ConfigurationManager.h"
#include "FlashStorage.h"

#include <esp_system.h>

ConfigurationManager::ConfigurationManager(const std::string &configNS) : m_ns(configNS)
{
}

const std::string &ConfigurationManager::getNamespace() const
{
    return m_ns;
}

esp_err_t ConfigurationManager::getConfiguration(const std::string &name, uint32_t &value) const
{
    return FlashStorage::get_u32(m_ns, name, value);
}

esp_err_t ConfigurationManager::getConfiguration(const std::string &name, int32_t &value) const
{
    return FlashStorage::get_i32(m_ns, name, value);
}

esp_err_t ConfigurationManager::getConfiguration(const std::string &name, uint64_t &value) const
{
    return FlashStorage::get_u64(m_ns, name, value);
}

esp_err_t ConfigurationManager::getConfiguration(const std::string &name, int64_t &value) const
{
    return FlashStorage::get_i64(m_ns, name, value);
}

esp_err_t ConfigurationManager::getConfiguration(const std::string &name, std::string &value) const
{
    return FlashStorage::get_string(m_ns, name, value);
}

esp_err_t ConfigurationManager::getConfiguration(const char *name, uint32_t &value) const
{
    return getConfiguration(std::string(name), value);
}

esp_err_t ConfigurationManager::getConfiguration(const char *name, int32_t &value) const
{
    return getConfiguration(std::string(name), value);
}

esp_err_t ConfigurationManager::getConfiguration(const char *name, uint64_t &value) const
{
    return getConfiguration(std::string(name), value);
}

esp_err_t ConfigurationManager::getConfiguration(const char *name, int64_t &value) const
{
    return getConfiguration(std::string(name), value);
}

esp_err_t ConfigurationManager::getConfiguration(const char *name, std::string &value) const
{
    return getConfiguration(std::string(name), value);
}

esp_err_t ConfigurationManager::setConfiguration(const std::string &name, const uint32_t value)
{
    return FlashStorage::set_u32(m_ns, name, value);
}

esp_err_t ConfigurationManager::setConfiguration(const std::string &name, const int32_t value)
{
    return FlashStorage::set_i32(m_ns, name, value);
}

esp_err_t ConfigurationManager::setConfiguration(const std::string &name, const uint64_t value)
{
    return FlashStorage::set_u64(m_ns, name, value);
}

esp_err_t ConfigurationManager::setConfiguration(const std::string &name, const int64_t value)
{
    return FlashStorage::set_i64(m_ns, name, value);
}

esp_err_t ConfigurationManager::setConfiguration(const std::string &name, const std::string &value)
{
    return FlashStorage::set_string(m_ns, name, value);
}

esp_err_t ConfigurationManager::setConfiguration(const char *name, const uint32_t value)
{
    return setConfiguration(std::string(name), value);
}

esp_err_t ConfigurationManager::setConfiguration(const char *name, const int32_t value)
{
    return setConfiguration(std::string(name), value);
}

esp_err_t ConfigurationManager::setConfiguration(const char *name, const uint64_t value)
{
    return setConfiguration(std::string(name), value);
}

esp_err_t ConfigurationManager::setConfiguration(const char *name, const int64_t value)
{
    return setConfiguration(std::string(name), value);
}

esp_err_t ConfigurationManager::setConfiguration(const char *name, const std::string &value)
{
    return setConfiguration(std::string(name), value);
}