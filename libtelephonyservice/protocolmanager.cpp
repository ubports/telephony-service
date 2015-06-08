#include "protocolmanager.h"

ProtocolManager *ProtocolManager::instance()
{
    static ProtocolManager self;
    return &self;
}

ProtocolManager::Protocols ProtocolManager::protocols() const
{

}

ProtocolManager::Protocols ProtocolManager::textProtocols() const
{

}

ProtocolManager::Protocols ProtocolManager::voiceProtocols() const
{

}

void ProtocolManager::loadSupportedProtocols()
{

}

ProtocolManager::ProtocolManager(QObject *parent) :
    QObject(parent)
{
    loadSupportedProtocols();
}
