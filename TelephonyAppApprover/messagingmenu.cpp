/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
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

#include "messagingmenu.h"
#include "contactmodel.h"
#include "contactentry.h"
#include <gio/gio.h>

MessagingMenu::MessagingMenu(QObject *parent) :
    QObject(parent)
{
    // create the messaging menu app
    mApp = messaging_menu_app_new("telephony-app.desktop");
    messaging_menu_app_register(mApp);
    messaging_menu_app_append_source(mApp, "telephony-app", g_icon_new_for_string("telephony-app", NULL), "Telephony App");
}

void MessagingMenu::addMessage(const QString &phoneNumber, const QString &messageId, const QDateTime &timestamp, const QString &text)
{
    // try to get a contact for that phone number
    ContactEntry *contact = ContactModel::instance()->contactFromPhoneNumber(phoneNumber);
    QString iconPath;
    QString contactAlias = phoneNumber;

    if (contact) {
        iconPath = contact->avatar().toLocalFile();
        contactAlias = contact->displayLabel();
    }

    GFile *file = g_file_new_for_path(iconPath.toUtf8().data());
    GIcon *icon = g_file_icon_new(file);
    MessagingMenuMessage *message = messaging_menu_message_new(messageId.toUtf8().data(),
                                                               icon,
                                                               contactAlias.toUtf8().data(),
                                                               NULL,
                                                               text.toUtf8().data(),
                                                               timestamp.toMSecsSinceEpoch());
    mMessages[messageId] = message;
    messaging_menu_app_append_message(mApp, message, "telephony-app", true);
    // TODO: setup callbacks

    g_object_unref(file);
    g_object_unref(icon);
}

void MessagingMenu::removeMessage(const QString &messageId)
{
    if (!mMessages.contains(messageId)) {
        return;
    }

    MessagingMenuMessage *message = mMessages.take(messageId);
    messaging_menu_app_remove_message(mApp, message);
    g_object_unref(message);
}

void MessagingMenu::addCall(const QString &phoneNumber, const QDateTime &timestamp)
{
    Call call;
    bool found = false;
    Q_FOREACH(Call callMessage, mCalls) {
        if (ContactModel::comparePhoneNumbers(callMessage.number, phoneNumber)) {
            call = callMessage;
            found = true;
            mCalls.removeOne(callMessage);

            // remove the previous entry and add a new one increasing the missed call count
            messaging_menu_app_remove_message(mApp, call.message);
            g_object_unref(call.message);
            break;
        }
    }

    if (!found) {
            ContactEntry *contact = ContactModel::instance()->contactFromPhoneNumber(phoneNumber);
            if (contact) {
                call.contactAlias = contact->displayLabel();
                call.contactIcon = contact->avatar().toLocalFile();
            } else {
                call.contactAlias = phoneNumber;
            }
            call.number = phoneNumber;
            call.count = 0;
    }

    call.count++;

    QString text;
    if (call.count > 1) {
        text = QString("%1 missed calls").arg(call.count);
    } else {
        text = QString("1 missed call");
    }
    GFile *file = g_file_new_for_path(call.contactIcon.toUtf8().data());
    GIcon *icon = g_file_icon_new(file);
    call.message = messaging_menu_message_new(call.number.toUtf8().data(),
                                              icon,
                                              call.contactAlias.toUtf8().data(),
                                              NULL,
                                              text.toUtf8().data(),
                                              timestamp.toMSecsSinceEpoch());
    messaging_menu_app_append_message(mApp, call.message, "telephony-app", true);
    mCalls.append(call);
    // TODO: setup actions and callbacks

    g_object_unref(file);
    g_object_unref(icon);
}

void MessagingMenu::showVoicemailEntry(int count)
{
}

void MessagingMenu::hideVoicemailEntry()
{
}


MessagingMenu *MessagingMenu::instance()
{
    static MessagingMenu *menu = new MessagingMenu();
    return menu;
}

MessagingMenu::~MessagingMenu()
{
    messaging_menu_app_unregister(mApp);
    g_object_unref(mApp);
}
