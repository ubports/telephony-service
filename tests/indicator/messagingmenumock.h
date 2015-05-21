#ifndef MESSAGINGMENUMOCK_H
#define MESSAGINGMENUMOCK_H

#include <QObject>

class MessagingMenuMock : public QObject
{
    Q_OBJECT
public:
    static MessagingMenuMock *instance();


Q_SIGNALS:
    void appCreated(const QString &desktopId);
    void appRegistered(const QString &desktopId);
    void sourceAdded(const QString &app, const QString &id, const QString &icon, const QString &label);

    void messageCreated(const QString &id,
                        const QString &icon,
                        const QString &title,
                        const QString &subtitle,
                        const QString &body,
                        const QDateTime &time);
    void messageAdded(const QString &app,
                      const QString &messageId,
                      const QString &sourceId,
                      bool notify);
    void messageRemoved(const QString &app,
                        const QString &id);
    void actionAdded(const QString &messageId,
                     const QString &actionId,
                     const QString &label);

private:
    explicit MessagingMenuMock(QObject *parent = 0);

};

#endif // MESSAGINGMENUMOCK_H
