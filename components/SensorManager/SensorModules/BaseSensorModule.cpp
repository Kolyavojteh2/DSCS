#include "BaseSensorModule.h"
#include <esp_log.h>

BaseSensorModule::BaseSensorModule(const std::string &name) : m_name(name) {}

const std::string &BaseSensorModule::getName() const
{
    return m_name;
}

void BaseSensorModule::configure()
{
    ESP_LOGW(this->getName().c_str(), "configure() not implemented.");
}
void BaseSensorModule::reset()
{
    ESP_LOGW(this->getName().c_str(), "reset() not implemented.");
}

void BaseSensorModule::powerOn()
{
    ESP_LOGW(this->getName().c_str(), "powerOn() not implemented.");
}
void BaseSensorModule::powerOff()
{
    ESP_LOGW(this->getName().c_str(), "powerOff() not implemented.");
}

void BaseSensorModule::clearData(const std::string & /*dataName*/, const time_t /*from*/, const time_t /*to*/)
{
    ESP_LOGW(this->getName().c_str(), "clearData() not implemented.");
}
