/*
 * Copyright (C) 2012 Canonical, Ltd.
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

#include "applicationutils.h"
#include "callmanager.h"
#include "config.h"
#include "contactutils.h"
#include "phoneutils.h"
#include "messagingmenu.h"
#include "telepathyhelper.h"
#include "accountentry.h"
#include <QContactAvatar>
#include <QContactFetchRequest>
#include <QContactFilter>
#include <QContactPhoneNumber>
#include <QDateTime>
#include <QDebug>
#include <gio/gio.h>
#include <messaging-menu-message.h>
#include <History/Manager>
#include <History/TextEvent>

namespace C {
#include <libintl.h>
}

#define SOURCE_ID "telephony-service-indicator"

QTCONTACTS_USE_NAMESPACE

MessagingMenu::MessagingMenu(QObject *parent) :
    QObject(parent)
{
    GIcon *icon = g_icon_new_for_string("telephony-service-indicator", NULL);
    mMessagesApp = messaging_menu_app_new("telephony-service-sms.desktop");
    messaging_menu_app_register(mMessagesApp);
    messaging_menu_app_append_source(mMessagesApp, SOURCE_ID, icon, C::gettext("Telephony Service"));

    mCallsApp = messaging_menu_app_new("telephony-service-call.desktop");
    messaging_menu_app_register(mCallsApp);
    messaging_menu_app_append_source(mCallsApp, SOURCE_ID, icon, C::gettext("Telephony Service"));

    g_object_unref(icon);
}

void MessagingMenu::addFlashMessage(const QString &senderId, const QString &accountId, const QString &messageId, const QDateTime &timestamp, const QString &text) {
    QUrl iconPath = QUrl::fromLocalFile(telephonyServiceDir() + "/assets/avatar-default@18.png");
    QString contactAlias = senderId;
    GFile *file = g_file_new_for_uri(iconPath.toString().toUtf8().data());
    GIcon *icon = g_file_icon_new(file);
 
    MessagingMenuMessage *message = messaging_menu_message_new(messageId.toUtf8().data(),
                                                               icon,
                                                               "",
                                                               NULL,
                                                               text.toUtf8().data(),
                                                               timestamp.toMSecsSinceEpoch() * 1000); // the value is expected to be in microseconds
    /* FIXME: uncomment when messaging-menu support two regular buttons
    messaging_menu_message_add_action(message,
                                      "saveFlashMessage",
                                      C::gettext("Save"),
                                      NULL,
                                      NULL
                                      );
    messaging_menu_message_add_action(message,
                                      "dismiss",
                                      C::gettext("Dismiss"),
                                      NULL,
                                      NULL
                                      );
    */
 
    g_signal_connect(message, "activate", G_CALLBACK(&MessagingMenu::flashMessageActivateCallback), this);
    QVariantMap details;
    details["senderId"] = senderId;
    details["accountId"] = accountId;
    details["messageId"] = messageId;
    details["timestamp"] = timestamp;
    details["text"] = text;
    mMessages[messageId] = details;
    messaging_menu_app_append_message(mMessagesApp, message, SOURCE_ID, true);

    g_object_unref(file);
    g_object_unref(icon);
    g_object_unref(message);
 
}

void MessagingMenu::addMessage(const QString &senderId, const QStringList &participantIds, const QString &accountId, const QString &messageId, const QDateTime &timestamp, const QString &text)
{
    // try to get a contact for that phone number
    QUrl iconPath = QUrl::fromLocalFile(telephonyServiceDir() + "/assets/avatar-default@18.png");
    QString contactAlias = senderId;

    // try to match the contact info
    QContactFetchRequest *request = new QContactFetchRequest(this);
    request->setFilter(QContactPhoneNumber::match(senderId));

    // place the messaging-menu item only after the contact fetch request is finished, as we can´t simply update
    QObject::connect(request, &QContactAbstractRequest::stateChanged,
                     [request, senderId, participantIds, accountId, messageId, text, timestamp, iconPath, contactAlias, this]() {
        // only process the results after the finished state is reached
        if (request->state() != QContactAbstractRequest::FinishedState) {
            return;
        }

        QString displayLabel;
        QUrl avatar;

        if (request->contacts().size() > 0) {
            QContact contact = request->contacts().at(0);
            displayLabel = ContactUtils::formatContactName(contact);
            avatar = contact.detail<QContactAvatar>().imageUrl();
        }

        if (displayLabel.isEmpty()) {
            displayLabel = contactAlias;
        }

        if (avatar.isEmpty()) {
            avatar = iconPath;
        }

        GFile *file = g_file_new_for_uri(avatar.toString().toUtf8().data());
        GIcon *icon = g_file_icon_new(file);
        MessagingMenuMessage *message = messaging_menu_message_new(messageId.toUtf8().data(),
                                                                   icon,
                                                                   displayLabel.toUtf8().data(),
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
        QVariantMap details;
        details["senderId"] = senderId;
        details["accountId"] = accountId;
        details["participantIds"] = participantIds;
 
        mMessages[messageId] = details;
        messaging_menu_app_append_message(mMessagesApp, message, SOURCE_ID, true);

        g_object_unref(file);
        g_object_unref(icon);
        g_object_unref(message);
    });

    request->setManager(ContactUtils::sharedManager());
    request->start();
}

void MessagingMenu::removeMessage(const QString &messageId)
{
    if (!mMessages.contains(messageId)) {
        return;
    }

    messaging_menu_app_remove_message_by_id(mMessagesApp, messageId.toUtf8().data());
    mMessages.remove(messageId);
}

void MessagingMenu::addCallToMessagingMenu(Call call, const QString &text)
{
    GVariant *messages = NULL;
    GFile *file = g_file_new_for_uri(call.contactIcon.toString().toUtf8().data());
    GIcon *icon = g_file_icon_new(file);
    MessagingMenuMessage *message = messaging_menu_message_new(call.number.toUtf8().data(),
                                                               icon,
                                                               call.contactAlias.toUtf8().data(),
                                                               NULL,
                                                               text.toUtf8().data(),
                                                               call.timestamp.toMSecsSinceEpoch() * 1000);  // the value is expected to be in microseconds

    call.messageId = messaging_menu_message_get_id(message);
    if (call.number != "x-ofono-private" && call.number != "x-ofono-unknown") {
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
        messages = g_variant_new_strv(predefinedMessages, -1);
        messaging_menu_message_add_action(message,
                                          "replyWithMessage",
                                          NULL, // label
                                          G_VARIANT_TYPE("s"),
                                          messages // predefined values
                                          );
    }
    g_signal_connect(message, "activate", G_CALLBACK(&MessagingMenu::callsActivateCallback), this);
    messaging_menu_app_append_message(mCallsApp, message, SOURCE_ID, true);
    mCalls.append(call);

    g_variant_unref(messages);
    g_object_unref(file);
    g_object_unref(icon);
    g_object_unref(message);
}

void MessagingMenu::addCall(const QString &phoneNumber, const QString &accountId, const QDateTime &timestamp)
{
    Call call;
    bool found = false;
    Q_FOREACH(Call callMessage, mCalls) {
        if (PhoneUtils::comparePhoneNumbers(callMessage.number, phoneNumber)) {
            call = callMessage;
            found = true;
            mCalls.removeOne(callMessage);

            // remove the previous entry and add a new one increasing the missed call count
            messaging_menu_app_remove_message_by_id(mCallsApp, callMessage.messageId.toUtf8().data());
            break;
        }
    }

    if (!found) {
        call.contactAlias = phoneNumber;
        call.accountId = accountId;
        call.contactIcon = QUrl::fromLocalFile(telephonyServiceDir() + "/assets/avatar-default@18.png");
        call.number = phoneNumber;
        call.count = 0;
    }

    call.count++;
    call.timestamp = timestamp;

    QString text;
    text = QString::fromUtf8(C::ngettext("%1 missed call", "%1 missed calls", call.count)).arg(call.count);

    if (phoneNumber.startsWith("x-ofono-private")) {
        call.contactAlias = C::gettext("Private number");
        addCallToMessagingMenu(call, text);
        return;
    } else if (phoneNumber.startsWith("x-ofono-unknown")) {
        call.contactAlias = C::gettext("Unknown number");
        addCallToMessagingMenu(call, text);
        return;
    }

    // try to match the contact info
    QContactFetchRequest *request = new QContactFetchRequest(this);
    request->setFilter(QContactPhoneNumber::match(phoneNumber));

    // place the messaging-menu item only after the contact fetch request is finished, as we can´t simply update
    QObject::connect(request, &QContactAbstractRequest::stateChanged, [request, call, text, this]() {
        // only process the results after the finished state is reached
        if (request->state() != QContactAbstractRequest::FinishedState) {
            return;
        }

        Call newCall = call;
        if (request->contacts().size() > 0) {
            QContact contact = request->contacts().at(0);
            QString displayLabel = ContactUtils::formatContactName(contact);
            QUrl avatar = contact.detail<QContactAvatar>().imageUrl();

            if (!displayLabel.isEmpty()) {
                newCall.contactAlias = displayLabel;
            }

            if (!avatar.isEmpty()) {
                newCall.contactIcon = avatar;
            }
        }
        addCallToMessagingMenu(newCall, text);
    });

    request->setManager(ContactUtils::sharedManager());
    request->start();
}

void MessagingMenu::showVoicemailEntry(AccountEntry *account)
{
    messaging_menu_app_remove_message_by_id(mCallsApp, account->accountId().toUtf8().data());
    mVoicemailIds.removeAll(account->accountId());

    QString messageBody = C::gettext("Voicemail messages");
    uint count = account->voicemailCount();
    if (count != 0) {
        messageBody = QString::fromUtf8(C::ngettext("%1 voicemail message", "%1 voicemail messages", count)).arg(count);
    }

    GIcon *icon = g_themed_icon_new("indicator-call");

    QString accountLabel(C::gettext("Voicemail"));
    if (TelepathyHelper::instance()->activeAccounts().size() > 1) {
        accountLabel += " - " + account->displayName();
    }

    MessagingMenuMessage *message = messaging_menu_message_new(account->accountId().toUtf8().data(),
                                                               icon,
                                                               accountLabel.toUtf8().data(),
                                                               NULL,
                                                               messageBody.toUtf8().data(),
                                                               QDateTime::currentDateTime().toMSecsSinceEpoch() * 1000); // the value is expected to be in microseconds
    g_signal_connect(message, "activate", G_CALLBACK(&MessagingMenu::callsActivateCallback), this);
    messaging_menu_app_append_message(mCallsApp, message, SOURCE_ID, true);
    mVoicemailIds.append(account->accountId());

    g_object_unref(icon);
    g_object_unref(message);
}

void MessagingMenu::hideVoicemailEntry(AccountEntry *account)
{
    mVoicemailIds.removeAll(account->accountId());
    messaging_menu_app_remove_message_by_id(mCallsApp, account->accountId().toUtf8().data());
}

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

void MessagingMenu::flashMessageActivateCallback(MessagingMenuMessage *message, const char *actionId, GVariant *param, MessagingMenu *instance)
{
    QString action(actionId);
    QString messageId(messaging_menu_message_get_id(message));

    if (action == "saveFlashMessage") {
        QMetaObject::invokeMethod(instance, "saveFlashMessage", Q_ARG(QString, messageId));
    }
}
void MessagingMenu::callsActivateCallback(MessagingMenuMessage *message, const char *actionId, GVariant *param, MessagingMenu *instance)
{
    QString action(actionId);
    QString messageId(messaging_menu_message_get_id(message));

    if (instance->mVoicemailIds.contains(messageId)) {
        QMetaObject::invokeMethod(instance, "callVoicemail", Q_ARG(QString, messageId));
        return;
    }
    if (action == "callBack" || action.isEmpty()) {
        QMetaObject::invokeMethod(instance, "callBack", Q_ARG(QString, messageId));
    } else if (action == "replyWithMessage") {
        QString text(g_variant_get_string(param, NULL));
        QMetaObject::invokeMethod(instance, "replyWithMessage", Q_ARG(QString, messageId), Q_ARG(QString, text));
    }
}

void MessagingMenu::sendMessageReply(const QString &messageId, const QString &reply)
{
    QString senderId = mMessages[messageId]["senderId"].toString();
    QString accountId = mMessages[messageId]["accountId"].toString();
    QStringList participantIds = mMessages[messageId]["participantIds"].toStringList();
    QStringList recipients;
    if (!senderId.isEmpty()) {
        recipients << senderId;
    }
    recipients << participantIds;
    recipients.removeDuplicates();
    Q_EMIT replyReceived(recipients, accountId, reply);

    Q_EMIT messageRead(recipients, accountId, messageId);
}

void MessagingMenu::saveFlashMessage(const QString &messageId)
{
    QVariantMap details = mMessages[messageId];
    History::Thread thread = History::Manager::instance()->threadForParticipants(details["accountId"].toString(),
                                                                                 History::EventTypeText,
                                                                                 QStringList() << details["senderId"].toString(),
                                                                                 History::MatchPhoneNumber,
                                                                                 true);
    History::TextEvent textEvent(details["accountId"].toString(),
                                 thread.threadId(), 
                                 details["messageId"].toString(), 
                                 details["senderId"].toString(),
                                 details["timestamp"].toDateTime(),
                                 false,
                                 details["text"].toString(),
                                 History::MessageTypeText);
    History::Events events;
    events.append(textEvent);
                                 
    History::Manager::instance()->writeEvents(events);
}

void MessagingMenu::showMessage(const QString &messageId)
{
    QString senderId = mMessages[messageId]["senderId"].toString();
    // FIXME: add support for mms group chat
    ApplicationUtils::openUrl(QString("message:///%1").arg(QString(QUrl::toPercentEncoding(senderId))));
}

void MessagingMenu::callBack(const QString &messageId)
{
    QString phoneNumber = callFromMessageId(messageId).number;
    qDebug() << "TelephonyService/MessagingMenu: Calling back" << phoneNumber;
    ApplicationUtils::openUrl(QString("tel:///%1").arg(QString(QUrl::toPercentEncoding(phoneNumber))));
}

void MessagingMenu::replyWithMessage(const QString &messageId, const QString &reply)
{
    Call call = callFromMessageId(messageId);
    qDebug() << "TelephonyService/MessagingMenu: Replying to call" << call.number << "with text" << reply;
    Q_EMIT replyReceived(QStringList() << call.number, call.accountId, reply);
}

void MessagingMenu::callVoicemail(const QString &messageId)
{
    QString voicemailNumber;
    Q_FOREACH(AccountEntry *accountEntry, TelepathyHelper::instance()->accounts()) {
        if (!accountEntry->voicemailNumber().isEmpty() && messageId == accountEntry->accountId()) {
            voicemailNumber = accountEntry->voicemailNumber();
            break;
        }
    }

    qDebug() << "TelephonyService/MessagingMenu: Calling voicemail for messageId" << messageId;
    if (!voicemailNumber.isEmpty()) {
        ApplicationUtils::openUrl(QUrl(QString("tel:///%1").arg(voicemailNumber)));
    }
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
