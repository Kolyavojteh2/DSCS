#ifndef CONFIGURATION_MANAGER_H
#define CONFIGURATION_MANAGER_H

#include <esp_err.h>

#include <vector>
#include <string>

#define SERVER_NAMESPACE_STR "server"
#define SERVER_CONFIG_IP_STR "ip"
#define SERVER_CONFIG_PORT_STR "port"

#define CHRONO_NAMESPACE_STR "chrono"
#define CHRONO_CONFIG_UPDATE_PERIOD_STR "update_period"

class ConfigurationManager
{
public:
    ConfigurationManager(const std::string &configNS);

    esp_err_t getConfiguration(const std::string &name, uint32_t &value) const;
    esp_err_t getConfiguration(const std::string &name, int32_t &value) const;
    esp_err_t getConfiguration(const std::string &name, uint64_t &value) const;
    esp_err_t getConfiguration(const std::string &name, int64_t &value) const;
    esp_err_t getConfiguration(const std::string &name, std::string &value) const;

    esp_err_t getConfiguration(const char *name, uint32_t &value) const;
    esp_err_t getConfiguration(const char *name, int32_t &value) const;
    esp_err_t getConfiguration(const char *name, uint64_t &value) const;
    esp_err_t getConfiguration(const char *name, int64_t &value) const;
    esp_err_t getConfiguration(const char *name, std::string &value) const;

    esp_err_t setConfiguration(const std::string &name, const uint32_t value);
    esp_err_t setConfiguration(const std::string &name, const int32_t value);
    esp_err_t setConfiguration(const std::string &name, const uint64_t value);
    esp_err_t setConfiguration(const std::string &name, const int64_t value);
    esp_err_t setConfiguration(const std::string &name, const std::string &value);

    esp_err_t setConfiguration(const char *name, const uint32_t value);
    esp_err_t setConfiguration(const char *name, const int32_t value);
    esp_err_t setConfiguration(const char *name, const uint64_t value);
    esp_err_t setConfiguration(const char *name, const int64_t value);
    esp_err_t setConfiguration(const char *name, const std::string &value);

    const std::string &getNamespace() const;

private:
    std::string m_ns;
};

#endif // CONFIGURATION_MANAGER_H