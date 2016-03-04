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
#include "ofonoaccountentry.h"
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

void MessagingMenu::addMessage(const QString &senderId, const QString &contactAlias, const QStringList &participantIds, const QString &accountId, const QString &messageId, const QDateTime &timestamp, const QString &text)
{
    // try to get a contact for that phone number
    QUrl iconPath = QUrl::fromLocalFile(telephonyServiceDir() + "/assets/avatar-default@18.png");

    AccountEntry *account = TelepathyHelper::instance()->accountForId(accountId);
    if (!account) {
        return;
    }

    // FIXME: for accounts not based on phone number, we need to match other fields.
    // Right now we don't even bother trying to match contact data

    // try to match the contact info
    QContactFetchRequest *request = new QContactFetchRequest(this);
    request->setFilter(QContactPhoneNumber::match(senderId));

    QVariantMap details;
    details["senderId"] = senderId;
    details["accountId"] = accountId;
    details["participantIds"] = participantIds;
 
    mMessages[messageId] = details;

    // place the messaging-menu item only after the contact fetch request is finished, as we can´t simply update
    QObject::connect(request, &QContactAbstractRequest::stateChanged,
                     [request, senderId, participantIds, accountId, messageId, text, timestamp, iconPath, contactAlias, this](QContactAbstractRequest::State newState) {

        GFile *file = NULL;
        GIcon *icon = NULL;

        // only process the results after the finished state is reached
        // also, if the ack happens before contacts service return the request we have to
        // simply skip this
        if (newState != QContactAbstractRequest::FinishedState || !mMessages.contains(messageId)) {
            return;
        }

        QString displayLabel;
        QString subTitle;
        QUrl avatar;

        if (senderId == OFONO_UNKNOWN_NUMBER) {
            displayLabel = C::gettext("Unknown number");
        } else if (request->contacts().size() > 0) {
            QContact contact = request->contacts().at(0);
            displayLabel = ContactUtils::formatContactName(contact);
            avatar = contact.detail<QContactAvatar>().imageUrl();
        }

        if (displayLabel.isEmpty()) {
            displayLabel = contactAlias;
        }

        if (participantIds.size() > 1) {
            displayLabel = QString::fromUtf8(C::gettext("Message to group from %1")).arg(displayLabel);
            avatar = QUrl::fromLocalFile(telephonyServiceDir() + "/assets/contact-group.svg");
        }

        AccountEntry::addAccountLabel(accountId, displayLabel);

        if (avatar.isEmpty()) {
            avatar = iconPath;
        }

        if (!icon) {
            file = g_file_new_for_uri(avatar.toString().toUtf8().data());
            icon = g_file_icon_new(file);
        }
        MessagingMenuMessage *message = messaging_menu_message_new(messageId.toUtf8().data(),
                                                                   icon,
                                                                   displayLabel.toUtf8().data(),
                                                                   subTitle.toUtf8().data(),
                                                                   text.toUtf8().data(),
                                                                   timestamp.toMSecsSinceEpoch() * 1000); // the value is expected to be in microseconds
        messaging_menu_message_add_action(message,
                                          "quickReply",
                                          C::gettext("Send"), // label
                                          G_VARIANT_TYPE("s"),
                                          NULL // predefined values
                                          );
        g_signal_connect(message, "activate", G_CALLBACK(&MessagingMenu::messageActivateCallback), this);

        messaging_menu_app_append_message(mMessagesApp, message, SOURCE_ID, true);

        if (file) {
            g_object_unref(file);
        }
        g_object_unref(icon);
        g_object_unref(message);
    });

    // FIXME: For accounts not based on phone numbers, don't try to match contacts for now
    if (account->type() == AccountEntry::PhoneAccount || account->type() == AccountEntry::MultimediaAccount) {
        request->setManager(ContactUtils::sharedManager());
        request->start();
    } else {
        // just emit the signal to pretend we did a contact search
        Q_EMIT request->stateChanged(QContactAbstractRequest::FinishedState);
    }
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
    MessagingMenuMessage *message = messaging_menu_message_new(call.targetId.toUtf8().data(),
                                                               icon,
                                                               call.contactAlias.toUtf8().data(),
                                                               NULL,
                                                               text.toUtf8().data(),
                                                               call.timestamp.toMSecsSinceEpoch() * 1000);  // the value is expected to be in microseconds

    call.messageId = messaging_menu_message_get_id(message);
    if (call.targetId != OFONO_PRIVATE_NUMBER && call.targetId != OFONO_UNKNOWN_NUMBER) {
        messaging_menu_message_add_action(message,
                                          "callBack",
                                          C::gettext("Call back"), // label
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
                                          C::gettext("Send"), // label
                                          G_VARIANT_TYPE("s"),
                                          messages // predefined values
                                          );
    }
    g_signal_connect(message, "activate", G_CALLBACK(&MessagingMenu::callsActivateCallback), this);
    messaging_menu_app_append_message(mCallsApp, message, SOURCE_ID, true);
    mCalls.append(call);

    g_object_unref(file);
    g_object_unref(icon);
    g_object_unref(message);
}

void MessagingMenu::addCall(const QString &targetId, const QString &accountId, const QDateTime &timestamp)
{
    Call call;
    bool found = false;
    AccountEntry *account = TelepathyHelper::instance()->accountForId(accountId);
    if (!account) {
        return;
    }

    Q_FOREACH(Call callMessage, mCalls) {
        // FIXME: we need a better strategy to group calls from different accounts
        if (account->compareIds(callMessage.targetId, targetId) && callMessage.accountId == accountId) {
            call = callMessage;
            found = true;
            mCalls.removeOne(callMessage);

            // remove the previous entry and add a new one increasing the missed call count
            messaging_menu_app_remove_message_by_id(mCallsApp, callMessage.messageId.toUtf8().data());
            break;
        }
    }

    if (!found) {
        call.contactAlias = targetId;
        call.accountId = accountId;
        call.contactIcon = QUrl::fromLocalFile(telephonyServiceDir() + "/assets/avatar-default@18.png");
        call.targetId = targetId;
        call.count = 0;
    }

    call.count++;
    call.timestamp = timestamp;

    QString text;
    text = QString::fromUtf8(C::ngettext("%1 missed call", "%1 missed calls", call.count)).arg(call.count);

    AccountEntry::addAccountLabel(accountId, text);

    if (targetId.startsWith(OFONO_PRIVATE_NUMBER)) {
        call.contactAlias = C::gettext("Private number");
        addCallToMessagingMenu(call, text);
        return;
    } else if (targetId.startsWith(OFONO_UNKNOWN_NUMBER)) {
        call.contactAlias = C::gettext("Unknown number");
        addCallToMessagingMenu(call, text);
        return;
    }

    // FIXME: we need to match other fields for accounts not based on phone numbers.
    // For now we are not even trying to match contact data

    // try to match the contact info
    QContactFetchRequest *request = new QContactFetchRequest(this);
    request->setFilter(QContactPhoneNumber::match(targetId));

    //FIXME: on arm64 the connect() statement below fails at runtime with the following output:
    //QObject::connect: signal not found in QtContacts::QContactFetchRequest
    // so we just disable it
#ifndef __aarch64__
    // place the messaging-menu item only after the contact fetch request is finished, as we can´t simply update
    QObject::connect(request, &QContactAbstractRequest::stateChanged, [request, call, text, this]() {
        // only process the results after the finished state is reached
        if (request->state() != QContactAbstractRequest::FinishedState) {
            return;
        }
#endif
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
#ifndef __aarch64__
    });


    // FIXME: For accounts not based on phone numbers, don't try to match contacts for now
    if (account->type() == AccountEntry::PhoneAccount) {
        request->setManager(ContactUtils::sharedManager());
        request->start();
    } else {
        // just emit the signal to pretend we did a contact search
        Q_EMIT request->stateChanged(QContactAbstractRequest::FinishedState);
    }
#endif
}

void MessagingMenu::removeCall(const QString &targetId, const QString &accountId)
{
    Call call;
    bool found = false;
    AccountEntry *account = TelepathyHelper::instance()->accountForId(accountId);
    if (!account) {
        qWarning() << "Account not found for id" << accountId;
        return;
    }

    Q_FOREACH(Call callMessage, mCalls) {
        // FIXME: we need a better strategy to group calls from different accounts
        if (account->compareIds(callMessage.targetId, targetId) && callMessage.accountId == accountId) {
            call = callMessage;
            found = true;
            mCalls.removeOne(callMessage);

            // remove the previous entry and add a new one increasing the missed call count
            messaging_menu_app_remove_message_by_id(mCallsApp, callMessage.messageId.toUtf8().data());
            break;
        }
    }
}

void MessagingMenu::showVoicemailEntry(AccountEntry *account)
{
    OfonoAccountEntry *ofonoAccount = qobject_cast<OfonoAccountEntry*>(account);
    if (!ofonoAccount) {
        return;
    }

    messaging_menu_app_remove_message_by_id(mCallsApp, account->accountId().toUtf8().data());
    mVoicemailIds.removeAll(account->accountId());

    QString messageBody = C::gettext("Voicemail messages");
    uint count = ofonoAccount->voicemailCount();
    if (count != 0) {
        messageBody = QString::fromUtf8(C::ngettext("%1 voicemail message", "%1 voicemail messages", count)).arg(count);
    }

    GIcon *icon = g_themed_icon_new("indicator-call");

    QString accountLabel(C::gettext("Voicemail"));
    AccountEntry::addAccountLabel(account->accountId(), accountLabel);

    MessagingMenuMessage *message = messaging_menu_message_new(account->accountId().toUtf8().data(),
                                                               icon,
                                                               accountLabel.toUtf8().data(),
                                                               NULL,
                                                               messageBody.toUtf8().data(),
                                                               QDateTime::currentDateTime().toMSecsSinceEpoch() * 1000); // the value is expected to be in microseconds
    g_signal_connect(message, "activate", G_CALLBACK(&MessagingMenu::callsActivateCallback), this);
    messaging_menu_app_append_message(mCallsApp, message, SOURCE_ID, true);
    mVoicemailIds.append(ofonoAccount->accountId());

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
    QVariantMap message = mMessages[messageId];
    QString senderId = message["senderId"].toString();
    QString accountId = message["accountId"].toString();
    QStringList participantIds = message["participantIds"].toStringList();
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
    AccountEntry *account = TelepathyHelper::instance()->accountForId(details["accountId"].toString());
    bool phoneNumberBased = account && (account->type() == AccountEntry::PhoneAccount);
    History::Thread thread = History::Manager::instance()->threadForParticipants(details["accountId"].toString(),
                                                                                 History::EventTypeText,
                                                                                 QStringList() << details["senderId"].toString(),
                                                                                 phoneNumberBased ? History::MatchPhoneNumber :
                                                                                                    History::MatchCaseSensitive,
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
    QVariantMap message = mMessages[messageId];
    QString senderId = message["senderId"].toString();
    QString accountId = message["accountId"].toString();
    QStringList participantIds = message["participantIds"].toStringList();
    QStringList recipients;
    if (!senderId.isEmpty()) {
        recipients << senderId;
    }
    recipients << participantIds;
    recipients.removeDuplicates();

    QString url(QString("message:///%1").arg(QString(QUrl::toPercentEncoding(recipients.join(";")))));
    AccountEntry *account = TelepathyHelper::instance()->accountForId(accountId);
    if (account && account->type() == AccountEntry::GenericAccount) {
        url += QString("?accountId=%1").arg(QString(QUrl::toPercentEncoding(accountId)));
    }
 
    ApplicationUtils::openUrl(url);
}

void MessagingMenu::callBack(const QString &messageId)
{
    Call call = callFromMessageId(messageId);
    AccountEntry *account = TelepathyHelper::instance()->accountForId(call.accountId);
    if (!account) {
        qWarning() << "Could not find the account originating the call";
    }
    qDebug() << "TelephonyService/MessagingMenu: Calling back" << call.targetId;
    // FIXME: support accounts not based on phone numbers
    if (account->type() == AccountEntry::PhoneAccount || account->type() == AccountEntry::MultimediaAccount) {
        ApplicationUtils::openUrl(QString("tel:///%1").arg(QString(QUrl::toPercentEncoding(call.targetId))));
    }
}

void MessagingMenu::replyWithMessage(const QString &messageId, const QString &reply)
{
    Call call = callFromMessageId(messageId);
    qDebug() << "TelephonyService/MessagingMenu: Replying to call" << call.targetId << "with text" << reply;
    Q_EMIT replyReceived(QStringList() << call.targetId, call.accountId, reply);
}

void MessagingMenu::callVoicemail(const QString &messageId)
{
    QString voicemailNumber;
    // get the corresponding account
    OfonoAccountEntry *ofonoAccount = qobject_cast<OfonoAccountEntry*>(TelepathyHelper::instance()->accountForId(messageId));
    if (ofonoAccount) {
        voicemailNumber = ofonoAccount->voicemailNumber();
    }

    qDebug() << "TelephonyService/MessagingMenu: Calling voicemail for messageId" << messageId;
    if (!voicemailNumber.isEmpty()) {
        // FIXME: we need to specify which account to use
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
