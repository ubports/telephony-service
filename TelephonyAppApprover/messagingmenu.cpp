#include "messagingmenu.h"
#include "contactmodel.h"
#include "contactentry.h"

MessagingMenu::MessagingMenu(QObject *parent) :
    QObject(parent)
{
    // create the messaging menu app
    mApp = messaging_menu_app_new("telephony-app.desktop");
    messaging_menu_app_register(mApp);
    // TODO: add icon and check label
    messaging_menu_app_append_source(mApp, "telephony-app", NULL, "Telephony App");
}

void MessagingMenu::addMessage(const QString &phoneNumber, const QString &messageId, const QDateTime &timestamp, const QString &text)
{
    // try to get a contact for that phone number
    ContactEntry *contact = ContactModel::instance()->contactFromPhoneNumber(phoneNumber);
    QString icon;
    QString contactAlias = phoneNumber;

    if (contact) {
        icon = contact->avatar().path();
        contactAlias = contact->displayLabel();
    }

    MessagingMenuMessage *message = messaging_menu_message_new(messageId.toLocal8Bit().data(),
                                                               NULL,
                                                               contactAlias.toLocal8Bit().data(),
                                                               NULL,
                                                               text.toLocal8Bit().data(),
                                                               timestamp.toMSecsSinceEpoch());
    mMessages[messageId] = message;
    messaging_menu_app_append_message(mApp, message, "telephony-app", true);
}

void MessagingMenu::removeMessage(const QString &messageId)
{
    qDebug() << "Going to remove message" << messageId;
    if (!mMessages.contains(messageId)) {
        return;
    }

    MessagingMenuMessage *message = mMessages.take(messageId);
    messaging_menu_app_remove_message(mApp, message);
    g_object_unref(message);
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
