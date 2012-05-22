#ifndef TELEPATHYMANAGER_H
#define TELEPATHYMANAGER_H

#include <QObject>
#include <TelepathyQt4/AccountManager>
#include <TelepathyQt4/Contact>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/ConnectionManager>
#include <TelepathyQt4/Types>
#include "channelhandler.h"

class TelepathyManager : public QObject
{
    Q_OBJECT

public:
    explicit TelepathyManager(QObject *parent = NULL);
    ~TelepathyManager();

    static TelepathyManager *instance();

public Q_SLOTS:
    void initializeChannelHandler();
    void registerClients(void);

private:
    static TelepathyManager *mTelepathyManager;

    Tp::AccountManagerPtr mAccountManager;
    Tp::Features mAccountManagerFeatures;
    Tp::Features mAccountFeatures;
    Tp::Features mContactFeatures;
    Tp::Features mConnectionFeatures;
    ChannelHandler *mChannelHandler;
    Tp::ClientRegistrarPtr mClientRegistrar;
};

#endif // TELEPATHYMANAGER_H
