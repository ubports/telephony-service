#ifndef VOICEMAILINDICATOR_H
#define VOICEMAILINDICATOR_H

#include <QDBusInterface>
#include <QVariantMap>
#include <QStringList>

namespace QIndicate {
    class Indicator;
    class Server;
}

class VoiceMailIndicator : public QObject
{
    Q_OBJECT
public:
    explicit VoiceMailIndicator(QObject *parent = 0);

protected:
    void updateCounter();

public slots:
    void onPropertiesChanged(const QString &interfaceName,
                             const QVariantMap &changedProperties,
                             const QStringList &invalidatedProperties);
    void onIndicatorDisplay(QIndicate::Indicator *indicator);

private:
    QDBusConnection mConnection;
    QIndicate::Server *mIndicateServer;
    QIndicate::Indicator *mIndicator;
    QDBusInterface mInterface;
};

#endif // VOICEMAILINDICATOR_H
