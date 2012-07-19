/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TELEPATHYHELPER_H
#define TELEPATHYHELPER_H

#include <QObject>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/Contact>
#include <TelepathyQt/Connection>
#include <TelepathyQt/ConnectionManager>
#include <TelepathyQt/Types>
#include "channelhandler.h"
#include "channelobserver.h"
#include "chatmanager.h"
#include "callmanager.h"

class TelepathyHelper : public QObject
{
    Q_OBJECT

public:
    ~TelepathyHelper();

    static TelepathyHelper *instance();
    ChatManager *chatManager() const;
    CallManager *callManager() const;
    Tp::AccountPtr account() const;
    ChannelHandler *channelHandler() const;
    ChannelObserver *channelObserver() const;

Q_SIGNALS:
    void channelHandlerCreated(ChannelHandler *handler);
    void channelObserverCreated(ChannelObserver *observer);
    void accountReady();

public Q_SLOTS:
    void initializeTelepathyClients();
    void registerClients(void);

protected:
    void createAccount();
    void initializeAccount();
    void ensureAccountEnabled();
    void ensureAccountConnected();

private Q_SLOTS:
    void onAccountManagerReady(Tp::PendingOperation *op);
    void onAccountCreated(Tp::PendingOperation *op);
    void onAccountEnabled(Tp::PendingOperation *op);
    void onAccountStateChanged(bool enabled);
    void onAccountConnectionChanged(const Tp::ConnectionPtr &connection);

private:
    explicit TelepathyHelper(QObject *parent = 0);
    Tp::AccountManagerPtr mAccountManager;
    Tp::Features mAccountManagerFeatures;
    Tp::Features mAccountFeatures;
    Tp::Features mContactFeatures;
    Tp::Features mConnectionFeatures;
    Tp::ClientRegistrarPtr mClientRegistrar;
    Tp::AccountPtr mAccount;
    ChannelHandler *mChannelHandler;
    ChannelObserver *mChannelObserver;
    ChatManager *mChatManager;
    CallManager *mCallManager;
    bool mFirstTime;
};

#endif // TELEPATHYHELPER_H
