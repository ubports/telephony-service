#ifndef VOICEMAILINDICATOR_H
#define VOICEMAILINDICATOR_H

#include <QDBusInterface>
#include <QVariantMap>
#include <QStringList>

namespace QIndicate {
    class Indicator;
    class Server;
}

class VoiceMailIndicator : public QDBusInterface
{
    Q_OBJECT
public:
    explicit VoiceMailIndicator(QObject *parent = 0);

protected:
    void updateCounter();

private slots:
    void onPropertiesChanged(const QString &interfaceName,
                             const QVariantMap &changedProperties,
                             const QStringList &invalidatedProperties);
    void onIndicatorDisplay(QIndicate::Indicator *indicator);
private:
    QIndicate::Server *mIndicateServer;
    QIndicate::Indicator *mIndicator;
};

#endif // VOICEMAILINDICATOR_H
