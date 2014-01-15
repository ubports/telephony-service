#ifndef HANDLERCONTROLLER_H
#define HANDLERCONTROLLER_H

#include <QObject>
#include <QDBusInterface>

class HandlerController : public QObject
{
    Q_OBJECT
public:
    static HandlerController *instance();

public Q_SLOTS:
    void startCall(const QString &number);
    void hangUpCall(const QString &objectPath);
    void setHold(const QString &objectPath, bool hold);
    void setMuted(const QString &objectPath, bool muted);
    void setSpeakerMode(const QString &objectPath, bool enabled);
    void sendDTMF(const QString &objectPath, const QString &key);

    void sendMessage(const QString &number, const QString &message);
    void acknowledgeMessages(const QString &number, const QStringList &messageIds);

private:
    explicit HandlerController(QObject *parent = 0);
    QDBusInterface mHandlerInterface;
};

#endif // HANDLERCONTROLLER_H
