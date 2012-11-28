#ifndef MESSAGINGMENU_H
#define MESSAGINGMENU_H

#include <QObject>
#include <QMap>
#include <messaging-menu.h>
#include <messaging-menu-message.h>

class Call
{
public:
    Call() : count(0), message(0) { }
    QString number;
    int count;
    QString contactAlias;
    QString contactIcon;
    MessagingMenuMessage *message;

    bool operator==(const Call &other) {
        return other.number == number;
    }
};

class MessagingMenu : public QObject
{
    Q_OBJECT
public:
    static MessagingMenu *instance();
    virtual ~MessagingMenu();

    void addMessage(const QString &phoneNumber, const QString &messageId, const QDateTime &timestamp, const QString &text);
    void removeMessage(const QString &messageId);

    void addCall(const QString &phoneNumber, const QDateTime &timestamp);

private:
    explicit MessagingMenu(QObject *parent = 0);

    MessagingMenuApp *mApp;
    QMap<QString, MessagingMenuMessage*> mMessages;
    QList<Call> mCalls;
};

#endif // MESSAGINGMENU_H
