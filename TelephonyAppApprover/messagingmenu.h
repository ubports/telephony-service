#ifndef MESSAGINGMENU_H
#define MESSAGINGMENU_H

#include <QObject>
#include <QMap>
#include <messaging-menu.h>
#include <messaging-menu-message.h>

class MessagingMenu : public QObject
{
    Q_OBJECT
public:
    static MessagingMenu *instance();
    virtual ~MessagingMenu();

    void addMessage(const QString &phoneNumber, const QString &messageId, const QDateTime &timestamp, const QString &text);
    void removeMessage(const QString &messageId);

private:
    explicit MessagingMenu(QObject *parent = 0);

    MessagingMenuApp *mApp;
    QMap<QString, MessagingMenuMessage*> mMessages;
};

#endif // MESSAGINGMENU_H
