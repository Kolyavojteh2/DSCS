#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <sys/time.h>

class TimeManager
{
public:
    static void setTime(const time_t &newTime);
    static time_t getTime();
};

#endif // TIME_MANAGER_H