#ifndef TELEPATHYHELPER_H
#define TELEPATHYHELPER_H

#include <QObject>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/Contact>
#include <TelepathyQt/Connection>
#include <TelepathyQt/ConnectionManager>
#include <TelepathyQt/Types>
#include "channelhandler.h"
#include "chatmanager.h"

class TelepathyHelper : public QObject
{
    Q_OBJECT

public:
    explicit TelepathyHelper(QObject *parent = NULL);
    ~TelepathyHelper();

    static TelepathyHelper *instance();
    ChatManager *chatManager() const;
    Tp::AccountPtr account() const;

Q_SIGNALS:
    void channelHandlerCreated(ChannelHandler *ch);

public Q_SLOTS:
    void initializeChannelHandler();
    void registerClients(void);

private Q_SLOTS:
    void onAccountManagerReady(Tp::PendingOperation *op);

private:
    static TelepathyHelper *mTelepathyHelper;

    Tp::AccountManagerPtr mAccountManager;
    Tp::Features mAccountManagerFeatures;
    Tp::Features mAccountFeatures;
    Tp::Features mContactFeatures;
    Tp::Features mConnectionFeatures;
    Tp::ClientRegistrarPtr mClientRegistrar;
    Tp::AccountPtr mAccount;
    ChannelHandler *mChannelHandler;
    ChatManager *mChatManager;
};

#endif // TELEPATHYHELPER_H
