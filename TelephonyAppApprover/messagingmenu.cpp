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
    QString icon;
    QString contactAlias = phoneNumber;

    if (contact) {
        icon = contact->avatar().toLocalFile();
        contactAlias = contact->displayLabel();
    }

    GFile *file = g_file_new_for_path(icon.toUtf8().data());
    MessagingMenuMessage *message = messaging_menu_message_new(messageId.toUtf8().data(),
                                                               g_file_icon_new(file),
                                                               contactAlias.toUtf8().data(),
                                                               NULL,
                                                               text.toUtf8().data(),
                                                               timestamp.toMSecsSinceEpoch());
    g_object_unref(file);
    mMessages[messageId] = message;
    messaging_menu_app_append_message(mApp, message, "telephony-app", true);
    // TODO: setup callbacks
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
    call.message = messaging_menu_message_new(call.number.toUtf8().data(),
                                              g_file_icon_new(file),
                                              call.contactAlias.toUtf8().data(),
                                              NULL,
                                              text.toUtf8().data(),
                                              timestamp.toMSecsSinceEpoch());
    messaging_menu_app_append_message(mApp, call.message, "telephony-app", true);
    mCalls.append(call);
    // TODO: setup actions and callbacks
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
