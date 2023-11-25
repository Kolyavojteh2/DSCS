#ifndef HEAP_MONITOR_H
#define HEAP_MONITOR_H

#include "../BaseSensorModule.h"

#include <map>

#define FREE_HEAP_STR "free_heap"

class HeapMonitor : public BaseSensorModule
{
public:
    HeapMonitor();

    void readData() override;
    void readFreeHeap(uint32_t &out);

    void getHeader(std::map<std::string, uint8_t> &header) override;
    void getData(const std::string &dataName,
                 std::vector<time_t> &dataTime, std::vector<uint8_t> &data,
                 unsigned int count,
                 const time_t from = 0, const time_t to = INT_LEAST32_MAX) override;
    void getSingleDataSize(const std::string &dataName, uint8_t &size) override;

    void clearData(const std::string &dataName, const time_t from = 0, const time_t to = INT_LEAST32_MAX) override;

private:
    void getFreeHeap(std::vector<time_t> &dataTime, std::vector<uint8_t> &data,
                     unsigned int count,
                     const time_t from = 0, const time_t to = INT_LEAST32_MAX);

    std::map<time_t, uint32_t> m_freeHeap;
};

#endif // HEAP_MONITOR_H
