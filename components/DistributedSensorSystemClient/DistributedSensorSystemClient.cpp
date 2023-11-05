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
    MeshDataExchangeModule::m_mechanismPacketHandlers.addHandler("bootstrapInRoot", DSSProtocolHandler::bootstrapRootHandler);
    MeshDataExchangeModule::m_mechanismPacketHandlers.addHandler("aliveNodeRequest", DSSProtocolHandler::aliveNodeRequestHandler);
    MeshDataExchangeModule::m_mechanismPacketHandlers.addHandler("chronoUpdateResponse", DSSProtocolHandler::chronoUpdateResponseHandler);
    MeshDataExchangeModule::m_mechanismPacketHandlers.addHandler("getConfigRequest", DSSProtocolHandler::getConfigRequestHandler);
    MeshDataExchangeModule::m_mechanismPacketHandlers.addHandler("setConfigRequest", DSSProtocolHandler::setConfigRequestHandler);

    MeshDataExchangeModule::m_mechanismPacketHandlers.addHandler("getSensorsPacketRequest", DSSProtocolHandler::getSensorsPacketRequestHandler);
    MeshDataExchangeModule::m_mechanismPacketHandlers.addHandler("getSensorHeaderPacketRequest", DSSProtocolHandler::getSensorHeaderPacketRequestHandler);
    MeshDataExchangeModule::m_mechanismPacketHandlers.addHandler("getSensorDataPacketRequest", DSSProtocolHandler::getSensorDataPacketRequestHandler);
    MeshDataExchangeModule::m_mechanismPacketHandlers.addHandler("clearSensorDataRequest", DSSProtocolHandler::clearSensorDataRequestHandler);
    MeshDataExchangeModule::m_mechanismPacketHandlers.addHandler("setSensorReadingMode", DSSProtocolHandler::setSensorReadingModeRequestHandler);
}
