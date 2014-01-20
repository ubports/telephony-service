/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of telephony-service.
 *
 * telephony-service is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-service is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "callnotification.h"
#include "contactutils.h"
#include "config.h"
#include <QContactAvatar>
#include <QContactFetchRequest>
#include <QContactPhoneNumber>
#include <libnotify/notify.h>
namespace C {
#include <libintl.h>
}

CallNotification::CallNotification(QObject *parent) :
    QObject(parent)
{
}

CallNotification *CallNotification::instance()
{
    static CallNotification *self = new CallNotification();
    return self;
}

void CallNotification::showNotificationForCall(const QStringList &participants, CallNotification::NotificationReason reason)
{
    QString title;
    bool isConference = participants.count() > 1;

    switch (reason) {
    case CallHeld:
        title = isConference? C::gettext("Conf call on hold") : C::gettext("%1 call on hold");
        break;
    case CallEnded:
        title = isConference? C::gettext("Conf call ended") : C::gettext("%1 call ended");
        break;
    case CallRejected:
        title = C::gettext("%1 call declined");
        break;
    }

    // just to make things easier, search for a contact even when the call is a conference
    QContactFetchRequest *request = new QContactFetchRequest(this);
    request->setFilter(QContactPhoneNumber::match(participants.first()));

    // place the notify-notification item only after the contact fetch request is finished, as we can´t simply update
    QObject::connect(request, &QContactAbstractRequest::stateChanged, [request, participants, title, isConference]() {
        QString finalTitle = title;
        // only process the results after the finished state is reached
        if (request->state() != QContactAbstractRequest::FinishedState) {
            return;
        }

        QString displayLabel;
        QString avatar;

        if (request->contacts().size() > 0) {
            QContact contact = request->contacts().at(0);
            displayLabel = ContactUtils::formatContactName(contact);
            avatar = contact.detail<QContactAvatar>().imageUrl().toEncoded();
        }

        if (!isConference) {
            finalTitle = title.arg(displayLabel.isEmpty() ? participants[0] : displayLabel);
        }

        if (avatar.isEmpty()) {
            avatar = QUrl(telephonyServiceDir() + "assets/avatar-default@18.png").toEncoded();
        }

        // show the notification
        NotifyNotification *notification = notify_notification_new(finalTitle.toStdString().c_str(),
                                                                   NULL,
                                                                   avatar.toStdString().c_str());

        GError *error = NULL;
        if (!notify_notification_show(notification, &error)) {
            qWarning() << "Failed to show message notification:" << error->message;
            g_error_free (error);
        }
    });

    request->setManager(ContactUtils::sharedManager());
    request->start();
}
