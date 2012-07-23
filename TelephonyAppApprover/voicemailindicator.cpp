#include "voicemailindicator.h"
#include <QDebug>
#include <QDBusReply>
#include <qindicateserver.h>
#include <qindicateindicator.h>

#define TELEPHONY_INTERFACE "com.canonical.android.telephony.Telephony"
#define MESSAGE_INDICATOR_PROPERTY "MsgWaitingInd"
#define MESSAGE_COUNT_PROPERTY "VMessageCount"

VoiceMailIndicator::VoiceMailIndicator(QObject *parent)
: QObject(parent),
  mConnection(QDBusConnection::sessionBus()),
  mInterface("com.canonical.Android",
             "/com/canonical/android/telephony/Telephony",
             "org.freedesktop.DBus.Properties",
             mConnection,
             this)
{
    mIndicateServer = new QIndicate::Server(this);
    mIndicateServer->setType("message");
    mIndicateServer->setDesktopFile("/usr/share/applications/telephony-app-phone.desktop");

    mIndicator = new QIndicate::Indicator(this);
    mIndicator->setNameProperty("Voicemail");
    connect(mIndicator,
            SIGNAL(display(QIndicate::Indicator*)),
            SLOT(onIndicatorDisplay(QIndicate::Indicator*)));

    QDBusReply<QVariant> reply = mInterface.call("Get", TELEPHONY_INTERFACE, MESSAGE_INDICATOR_PROPERTY);
    if (reply.isValid() && reply.value().toBool()) {
        mIndicator->show();
        mIndicator->setDrawAttentionProperty(true);
    }

    mConnection.connect(mInterface.service(), mInterface.path(), mInterface.interface(), QLatin1String("PropertiesChanged"),
                        this, SLOT(onPropertiesChanged(const QString&, const QVariantMap&, const QStringList&)));
}

void VoiceMailIndicator::updateCounter()
{
    QDBusReply<QVariant> reply = mInterface.call("Get", TELEPHONY_INTERFACE, MESSAGE_COUNT_PROPERTY);
    if (reply.isValid()) {
        mIndicator->setCountProperty(reply.value().toInt());
    } else {
        mIndicator->setCountProperty(0);
    }
}

void VoiceMailIndicator::onPropertiesChanged(const QString &interfaceName,
                                             const QVariantMap &changedProperties,
                                             const QStringList &invalidatedProperties)
{
    qDebug() << "Properties changed:" << interfaceName << changedProperties << invalidatedProperties;
    if (interfaceName != TELEPHONY_INTERFACE) {
        return;
    }

    if (changedProperties.contains(MESSAGE_INDICATOR_PROPERTY)) {
        bool visible = changedProperties[MESSAGE_INDICATOR_PROPERTY].toBool();
        if (visible) {
            updateCounter();
            mIndicator->show();
            mIndicator->setDrawAttentionProperty(true);
        } else {
            mIndicator->hide();
            mIndicator->setDrawAttentionProperty(false);
        }
    }

    if (invalidatedProperties.contains(MESSAGE_INDICATOR_PROPERTY)) {
        mIndicator->hide();
        mIndicator->setDrawAttentionProperty(false);
    }
}

void VoiceMailIndicator::onIndicatorDisplay(QIndicate::Indicator *indicator)
{
    QDBusInterface telephonyApp("com.canonical.TelephonyApp",
                                "/com/canonical/TelephonyApp",
                                "com.canonical.TelephonyApp");
    telephonyApp.call("ShowVoicemail");
}
