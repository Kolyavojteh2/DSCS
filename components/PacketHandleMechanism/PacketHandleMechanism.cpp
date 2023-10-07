#include "PacketHandleMechanism.h"
#include <esp_log.h>
#include <vector>

int PacketHandleMechanism::addHandler(const std::string &name, const PacketHandleFunc handler)
{
    // TODO: add overwrite handling error
    auto findIterator = m_handlers.find(name);
    if (findIterator != m_handlers.end())
        ESP_LOGW(moduleTag, "The handler \"%s\" is overwrited.", name.c_str());

    m_handlers[name] = handler;

    return 0;
}

int PacketHandleMechanism::removeHandler(const std::string &name)
{
    // TODO: add overwrite handling error
    auto findIterator = m_handlers.find(name);
    if (findIterator == m_handlers.end())
    {
        ESP_LOGE(moduleTag, "The handler \"%s\" doesn't exists.", name.c_str());
        return -1;
    }

    m_handlers.erase(name);

    return 0;
}

void PacketHandleMechanism::run(const std::vector<uint8_t> &bin)
{
    m_stopRunning = false;

    for (auto &handler : m_handlers)
    {
        if (m_stopRunning)
            break;

        handler.second(bin);
    }
}

void PacketHandleMechanism::stopIteration(void)
{
    m_stopRunning = true;
}