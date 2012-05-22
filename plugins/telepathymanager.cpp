#include "telepathymanager.h"

#include <TelepathyQt4/ClientRegistrar>

TelepathyManager *TelepathyManager::mTelepathyManager = 0;

TelepathyManager::TelepathyManager(QObject *parent)
    : QObject(parent),
      mChannelHandler(0)
{
    if (!mTelepathyManager) {
        mTelepathyManager = this;
    }

    mAccountFeatures << Tp::Account::FeatureCore
                     << Tp::Account::FeatureAvatar
                     << Tp::Account::FeatureProtocolInfo
                     << Tp::Account::FeatureCapabilities
                     << Tp::Account::FeatureProfile;
    mContactFeatures << Tp::Contact::FeatureAlias
                     << Tp::Contact::FeatureAvatarToken
                     << Tp::Contact::FeatureSimplePresence
                     << Tp::Contact::FeatureAvatarData
                     << Tp::Contact::FeatureCapabilities;
    mConnectionFeatures << Tp::Connection::FeatureCore
                        << Tp::Connection::FeatureSelfContact
                        << Tp::Connection::FeatureSimplePresence
                        << Tp::Connection::FeatureRoster
                        << Tp::Connection::FeatureRosterGroups;

    Tp::ChannelFactoryPtr channelFactory =
        Tp::ChannelFactory::create(QDBusConnection::sessionBus());

    mAccountManager = Tp::AccountManager::create(
            Tp::AccountFactory::create(QDBusConnection::sessionBus(), mAccountFeatures),
            Tp::ConnectionFactory::create(QDBusConnection::sessionBus(), mConnectionFeatures),
            channelFactory,
            Tp::ContactFactory::create(mContactFeatures));
}

TelepathyManager::~TelepathyManager()
{
}

TelepathyManager *TelepathyManager::instance()
{
    if (!mTelepathyManager) {
        mTelepathyManager = new TelepathyManager();
    }

    return mTelepathyManager;
}

void TelepathyManager::initializeChannelHandler()
{
    mChannelHandler = new ChannelHandler();
    Tp::AbstractClientPtr handler(mChannelHandler);
    mClientRegistrar->registerClient(handler, "TelephonyApp");
}

void TelepathyManager::registerClients()
{
    Tp::ChannelFactoryPtr channelFactory = Tp::ChannelFactoryPtr::constCast(mAccountManager->channelFactory());
    channelFactory->addCommonFeatures(Tp::Channel::FeatureCore);
    mClientRegistrar = Tp::ClientRegistrar::create(mAccountManager);
    initializeChannelHandler();
}
