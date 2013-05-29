/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
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

#include "config.h"
#include "phoneapputils.h"
#include "messagingmenu.h"
#include "contactmodel.h"
#include "contactentry.h"
#include <gio/gio.h>

namespace C {
#include <libintl.h>
}

#ifdef HAVE_MESSAGING_MENU_MESSAGE
    #include <messaging-menu-message.h>
#endif

MessagingMenu::MessagingMenu(QObject *parent) :
    QObject(parent), mVoicemailCount(-1)
{
    GIcon *icon = g_icon_new_for_string("phone-app", NULL);
    mMessagesApp = messaging_menu_app_new("phone-app-sms.desktop");
    messaging_menu_app_register(mMessagesApp);
    messaging_menu_app_append_source(mMessagesApp, "phone-app", icon, "Phone App");

    mCallsApp = messaging_menu_app_new("phone-app-call.desktop");
    messaging_menu_app_register(mCallsApp);
    messaging_menu_app_append_source(mCallsApp, "phone-app", icon, "Phone App");

    g_object_unref(icon);
}

void MessagingMenu::addMessage(const QString &phoneNumber, const QString &messageId, const QDateTime &timestamp, const QString &text)
{
#ifdef HAVE_MESSAGING_MENU_MESSAGE
    // try to get a contact for that phone number
    ContactEntry *contact = ContactModel::instance()->contactFromPhoneNumber(phoneNumber);
    QString iconPath;
    QString contactAlias = phoneNumber;

    if (contact) {
        iconPath = contact->avatar().toLocalFile();
        contactAlias = contact->displayLabel();
    }

    if (iconPath.isNull()) {
        iconPath = phoneAppDirectory() + "/assets/avatar-default@18.png";
    }

    GFile *file = g_file_new_for_path(iconPath.toUtf8().data());
    GIcon *icon = g_file_icon_new(file);
    MessagingMenuMessage *message = messaging_menu_message_new(messageId.toUtf8().data(),
                                                               icon,
                                                               contactAlias.toUtf8().data(),
                                                               NULL,
                                                               text.toUtf8().data(),
                                                               timestamp.toMSecsSinceEpoch() * 1000); // the value is expected to be in microseconds
    messaging_menu_message_add_action(message,
                                      "quickReply",
                                      NULL, // label
                                      G_VARIANT_TYPE("s"),
                                      NULL // predefined values
                                      );
    g_signal_connect(message, "activate", G_CALLBACK(&MessagingMenu::messageActivateCallback), this);

    // save the phone number to use in the actions
    mMessages[messageId] = phoneNumber;
    messaging_menu_app_append_message(mMessagesApp, message, "phone-app", true);

    g_object_unref(file);
    g_object_unref(icon);
    g_object_unref(message);
#endif
}

void MessagingMenu::removeMessage(const QString &messageId)
{
#ifdef HAVE_MESSAGING_MENU_MESSAGE
    if (!mMessages.contains(messageId)) {
        return;
    }

    messaging_menu_app_remove_message_by_id(mMessagesApp, messageId.toUtf8().data());
    mMessages.remove(messageId);
#endif
}

void MessagingMenu::addCall(const QString &phoneNumber, const QDateTime &timestamp)
{
#ifdef HAVE_MESSAGING_MENU_MESSAGE
    Call call;
    bool found = false;
    Q_FOREACH(Call callMessage, mCalls) {
        if (ContactModel::comparePhoneNumbers(callMessage.number, phoneNumber)) {
            call = callMessage;
            found = true;
            mCalls.removeOne(callMessage);

            // remove the previous entry and add a new one increasing the missed call count
            messaging_menu_app_remove_message_by_id(mCallsApp, callMessage.messageId.toUtf8().data());
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

    if (call.contactIcon.isEmpty()) {
        call.contactIcon = phoneAppDirectory() + "/assets/avatar-default@18.png";
    }

    QString text;
    text = QString::fromUtf8(C::ngettext("%1 missed call", "%1 missed calls", call.count)).arg(call.count);

    GFile *file = g_file_new_for_path(call.contactIcon.toUtf8().data());
    GIcon *icon = g_file_icon_new(file);
    MessagingMenuMessage *message = messaging_menu_message_new(call.number.toUtf8().data(),
                                                               icon,
                                                               call.contactAlias.toUtf8().data(),
                                                               NULL,
                                                               text.toUtf8().data(),
                                                               timestamp.toMSecsSinceEpoch() * 1000);  // the value is expected to be in microseconds
    call.messageId = messaging_menu_message_get_id(message);
    messaging_menu_message_add_action(message,
                                      "callBack",
                                      NULL, // label
                                      NULL, // argument type
                                      NULL // predefined values
                                      );
    const char *predefinedMessages[] = {
            C::gettext("I missed your call - can you call me now?"),
            C::gettext("I'm running late. I'm on my way."),
            C::gettext("I'm busy at the moment. I'll call you later."),
            C::gettext("I'll be 20 minutes late."),
            C::gettext("Sorry, I'm still busy. I'll call you later."),
            0
            };
    GVariant *messages = g_variant_new_strv(predefinedMessages, -1);
    messaging_menu_message_add_action(message,
                                      "replyWithMessage",
                                      NULL, // label
                                      G_VARIANT_TYPE("s"),
                                      messages // predefined values
                                      );
    g_signal_connect(message, "activate", G_CALLBACK(&MessagingMenu::callsActivateCallback), this);
    messaging_menu_app_append_message(mCallsApp, message, "phone-app", true);
    mCalls.append(call);

    g_object_unref(messages);
    g_object_unref(file);
    g_object_unref(icon);
    g_object_unref(message);
#endif
}

void MessagingMenu::showVoicemailEntry(int count)
{
#ifdef HAVE_MESSAGING_MENU_MESSAGE
    if (!mVoicemailId.isEmpty()) {
        // if the count didn't change, don't do anything
        if (count == mVoicemailCount) {
            return;
        }

        messaging_menu_app_remove_message_by_id(mCallsApp, mVoicemailId.toUtf8().data());
    }

    QString messageBody = C::gettext("Voicemail messages");
    if (count != 0) {
        messageBody = QString::fromUtf8(C::ngettext("%1 voicemail message", "%1 voicemail messages", count)).arg(count);
    }

    GIcon *icon = g_themed_icon_new("indicator-call");
    MessagingMenuMessage *message = messaging_menu_message_new("voicemail",
                                                               icon,
                                                               "Voicemail",
                                                               NULL,
                                                               messageBody.toUtf8().data(),
                                                               QDateTime::currentDateTime().toMSecsSinceEpoch() * 1000); // the value is expected to be in microseconds
    g_signal_connect(message, "activate", G_CALLBACK(&MessagingMenu::callsActivateCallback), this);
    mVoicemailId = "voicemail";

    g_object_unref(icon);
    g_object_unref(message);
#endif
}

void MessagingMenu::hideVoicemailEntry()
{
#ifdef HAVE_MESSAGING_MENU_MESSAGE
    if (!mVoicemailId.isEmpty()) {
        messaging_menu_app_remove_message_by_id(mCallsApp, mVoicemailId.toUtf8().data());
        mVoicemailId = "";
    }
#endif
}

#ifdef HAVE_MESSAGING_MENU_MESSAGE
void MessagingMenu::messageActivateCallback(MessagingMenuMessage *message, const char *actionId, GVariant *param, MessagingMenu *instance)
{
    QString action(actionId);
    QString messageId(messaging_menu_message_get_id(message));
    QString text(g_variant_get_string(param, NULL));

    if (action == "quickReply") {
        QMetaObject::invokeMethod(instance, "sendMessageReply", Q_ARG(QString, messageId), Q_ARG(QString, text));
    } else if (action.isEmpty()) {
        QMetaObject::invokeMethod(instance, "showMessage", Q_ARG(QString, messageId));
    }
}

void MessagingMenu::callsActivateCallback(MessagingMenuMessage *message, const char *actionId, GVariant *param, MessagingMenu *instance)
{
    QString action(actionId);
    QString messageId(messaging_menu_message_get_id(message));

    if (action == "callBack") {
        QMetaObject::invokeMethod(instance, "callBack", Q_ARG(QString, messageId));
    } else if (action == "replyWithMessage") {
        QString text(g_variant_get_string(param, NULL));
        QMetaObject::invokeMethod(instance, "replyWithMessage", Q_ARG(QString, messageId), Q_ARG(QString, text));
    } else if (messageId == instance->mVoicemailId) {
        QMetaObject::invokeMethod(instance, "callVoicemail", Q_ARG(QString, messageId));
    }
}
#endif

void MessagingMenu::sendMessageReply(const QString &messageId, const QString &reply)
{
    QString phoneNumber = mMessages[messageId];
    Q_EMIT replyReceived(phoneNumber, reply);

    Q_EMIT messageRead(phoneNumber, messageId);
}

void MessagingMenu::showMessage(const QString &messageId)
{
    QString phoneNumber = mMessages[messageId];

    PhoneAppUtils::instance()->startPhoneApp();
    QDBusInterface mPhoneAppInterface("com.canonical.PhoneApp",
                           "/com/canonical/PhoneApp",
                           "com.canonical.PhoneApp");

    mPhoneAppInterface.call("ShowMessages", phoneNumber);
}

void MessagingMenu::callBack(const QString &messageId)
{
    PhoneAppUtils::instance()->startPhoneApp();

    QString phoneNumber = callFromMessageId(messageId).number;
    qDebug() << "PhoneApp/MessagingMenu: Calling back" << phoneNumber;
    QDBusInterface mPhoneAppInterface("com.canonical.PhoneApp",
                           "/com/canonical/PhoneApp",
                           "com.canonical.PhoneApp");

    mPhoneAppInterface.call("CallNumber", phoneNumber);
}

void MessagingMenu::replyWithMessage(const QString &messageId, const QString &reply)
{
    QString phoneNumber = callFromMessageId(messageId).number;
    qDebug() << "PhoneApp/MessagingMenu: Replying to call" << phoneNumber << "with text" << reply;
    Q_EMIT replyReceived(phoneNumber, reply);
}

void MessagingMenu::callVoicemail(const QString &messageId)
{
    qDebug() << "PhoneApp/MessagingMenu: Calling voicemail for messageId" << messageId;
    PhoneAppUtils::instance()->startPhoneApp();
    QDBusInterface mPhoneAppInterface("com.canonical.PhoneApp",
                           "/com/canonical/PhoneApp",
                           "com.canonical.PhoneApp");

    mPhoneAppInterface.call("ShowVoicemail");
}

Call MessagingMenu::callFromMessageId(const QString &messageId)
{
    Q_FOREACH(const Call &call, mCalls) {
        if (call.messageId == messageId) {
            return call;
        }
     }
    return Call();
}


MessagingMenu *MessagingMenu::instance()
{
    static MessagingMenu *menu = new MessagingMenu();
    return menu;
}

MessagingMenu::~MessagingMenu()
{
    g_object_unref(mMessagesApp);
    g_object_unref(mCallsApp);
}
