/*
 * Copyright (C) 2012-2013 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 *
 * This file is part of phone-app.
 *
 * phone-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * phone-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "callhandler.h"
#include "contactmodel.h"
#include "telepathyhelper.h"
#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingContacts>

#define CANONICAL_IFACE_TELEPHONY "com.canonical.Telephony"

typedef QMap<QString, QVariant> dbusQMap;
Q_DECLARE_METATYPE(dbusQMap)

CallHandler::CallHandler(QObject *parent)
: QObject(parent)
{
}

void CallHandler::startCall(const QString &phoneNumber)
{
    // check if we are already talking to that phone number
    // FIXME: reimplement using objectPath

    // Request the contact to start audio call
    Tp::AccountPtr account = TelepathyHelper::instance()->account();
    if (account->connection() == NULL) {
        return;
    }

    connect(account->connection()->contactManager()->contactsForIdentifiers(QStringList() << phoneNumber),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onContactsAvailable(Tp::PendingOperation*)));
}

void CallHandler::onCallChannelAvailable(Tp::CallChannelPtr channel)
{
    // FIXME: check what more to do with the call
    mCallChannels.append(channel);
}

void CallHandler::onContactsAvailable(Tp::PendingOperation *op)
{
    Tp::PendingContacts *pc = qobject_cast<Tp::PendingContacts*>(op);

    if (!pc) {
        qCritical() << "The pending object is not a Tp::PendingContacts";
        return;
    }

    Tp::AccountPtr account = TelepathyHelper::instance()->account();

    // start call to the contacts
    Q_FOREACH(Tp::ContactPtr contact, pc->contacts()) {
        account->ensureAudioCall(contact, QLatin1String("audio"), QDateTime::currentDateTime(), "org.freedesktop.Telepathy.Client.PhoneApp");

        // hold the ContactPtr to make sure its refcounting stays bigger than 0
        mContacts[contact->id()] = contact;
    }
}
