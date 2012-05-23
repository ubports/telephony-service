#ifndef CHATMANAGER_H
#define CHATMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/Contact>

class ChatManager : public QObject
{
    Q_OBJECT
public:
    explicit ChatManager(QObject *parent = 0);
    
    Q_INVOKABLE bool isChattingToContact(const QString &contactId);
    Q_INVOKABLE void startChat(const QString &contactId);
    Q_INVOKABLE void endChat(const QString &contactId);

    Q_INVOKABLE void sendMessage(const QString &contactId, const QString &message);

signals:
    void chatReady(const QString &contactId);

public Q_SLOTS:
    void onTextChannelAvailable(Tp::TextChannelPtr channel);
    void onContactsAvailable(Tp::PendingOperation *op);


private:
    QMap<QString, Tp::TextChannelPtr> mChannels;
    QMap<QString, Tp::ContactPtr> mContacts;
};

#endif // CHATMANAGER_H
