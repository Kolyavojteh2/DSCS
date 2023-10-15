#include "TimeManager.h"
#include <sys/time.h>

void TimeManager::setTime(const time_t &newTime)
{
    struct timeval now;
    now.tv_sec = newTime;
    now.tv_usec = 0;

    int ret = settimeofday(&now, NULL);
    if (ret)
    {
        // TODO: add error handle
    }
}

time_t TimeManager::getTime()
{
    return time(NULL);
}
