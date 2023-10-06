#ifndef DISTRIBUTED_SENSOR_SYSTEM_CLIENT_H
#define DISTRIBUTED_SENSOR_SYSTEM_CLIENT_H

#include <vector>

#include <esp_netif.h>
#include <esp_log.h>
#include <esp_mesh.h>

#include "PacketHandleMechanism.h"

class DistributedSensorSystemClient
{
public:
    static DistributedSensorSystemClient &getInstance(void);

private:
    DistributedSensorSystemClient();
    DistributedSensorSystemClient(const DistributedSensorSystemClient &) = delete;
    DistributedSensorSystemClient &operator=(const DistributedSensorSystemClient &) = delete;

    void initHandlers();
};

#endif // DISTRIBUTED_SENSOR_SYSTEM_CLIENT_H