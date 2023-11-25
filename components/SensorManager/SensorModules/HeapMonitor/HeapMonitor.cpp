#include "HeapMonitor.h"
#include "TimeManager.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_heap_caps.h>

#define I2C_DELAY_FOR_SINGLE_WRITING 1000

HeapMonitor::HeapMonitor() : BaseSensorModule("HeapMonitor") {}

void HeapMonitor::readFreeHeap(uint32_t &out)
{
    out = heap_caps_get_free_size(MALLOC_CAP_8BIT);
}

void HeapMonitor::readData()
{
    uint32_t freeHeap;
    readFreeHeap(freeHeap);

    time_t currentTime = TimeManager::getTime();

    m_freeHeap[currentTime] = freeHeap;

    ESP_LOGI(getName().c_str(), "Time: %llu, FreeHeap: %lu", currentTime, freeHeap);
}

void HeapMonitor::getHeader(std::map<std::string, uint8_t> &header)
{
    header[FREE_HEAP_STR] = sizeof(uint32_t);
}

void HeapMonitor::getData(const std::string &dataName,
                          std::vector<time_t> &dataTime, std::vector<uint8_t> &data,
                          unsigned int count,
                          const time_t from, const time_t to)
{
    if (dataName == FREE_HEAP_STR)
        getFreeHeap(dataTime, data, count, from, to);
}

void HeapMonitor::getSingleDataSize(const std::string &dataName, uint8_t &size)
{
    std::map<std::string, uint8_t> header;
    getHeader(header);

    if (header.find(dataName) == header.end())
    {
        size = 0;
        return;
    }

    size = sizeof(time_t) + header[dataName];
}

void HeapMonitor::getFreeHeap(std::vector<time_t> &dataTime, std::vector<uint8_t> &data,
                              unsigned int count,
                              const time_t from, const time_t to)
{
    for (auto &it : m_freeHeap)
    {
        if (count == 0)
            break;

        if (it.first < from || it.first > to)
            continue;

        dataTime.push_back(it.first);
        data.push_back(it.second & 0xFF);
        data.push_back(it.second >> 8);
        data.push_back(it.second >> 16);
        data.push_back(it.second >> 24);

        count--;
    }
}

void HeapMonitor::clearData(const std::string &dataName, const time_t from, const time_t to)
{
    std::map<time_t, uint32_t> *ptrData = nullptr;

    if (dataName == FREE_HEAP_STR)
        ptrData = &m_freeHeap;

    if (ptrData == nullptr)
        return;

    for (auto it = ptrData->begin(); it != ptrData->end();)
    {
        if (it->first >= from && it->first <= to)
            it = ptrData->erase(it);
        else
            ++it;
    }
}