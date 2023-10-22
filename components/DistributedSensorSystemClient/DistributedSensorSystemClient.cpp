#include "DistributedSensorSystemClient.h"
#include "DSSProtocolHandler.h"
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
    // MeshDataExchangeModule::m_mechanismFromExternalIP.addHandler("aliveNodeRequest", DSSProtocolHandler::aliveNodeRequestHandler);

    MeshDataExchangeModule::m_mechanismPacketHandlers.addHandler("bootstrapInRoot", DSSProtocolHandler::bootstrapRootHandler);
    MeshDataExchangeModule::m_mechanismPacketHandlers.addHandler("aliveNodeRequest", DSSProtocolHandler::aliveNodeRequestHandler);
    MeshDataExchangeModule::m_mechanismPacketHandlers.addHandler("chronoUpdateResponse", DSSProtocolHandler::chronoUpdateResponseHandler);
    MeshDataExchangeModule::m_mechanismPacketHandlers.addHandler("getConfigRequest", DSSProtocolHandler::getConfigRequestHandler);
    MeshDataExchangeModule::m_mechanismPacketHandlers.addHandler("setConfigRequest", DSSProtocolHandler::setConfigRequestHandler);
}
