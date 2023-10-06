#include "DistributedSensorSystemClient.h"
#include "ExchangeProtocolHandler.h"
#include "MeshDataExchangeModule.h"

DistributedSensorSystemClient &DistributedSensorSystemClient::getInstance()
{
    static DistributedSensorSystemClient instance;

    return instance;
}

DistributedSensorSystemClient::DistributedSensorSystemClient()
{
    initHandlers();
}

void DistributedSensorSystemClient::initHandlers()
{
    MeshDataExchangeModule::m_mechanismFromExternalIP.addHandler("aliveNodeRequest", ExchangeProtocolHandler::aliveNodeRequestHandler);
}
