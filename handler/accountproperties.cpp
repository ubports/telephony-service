#include "accountproperties.h"
#include "telepathyhelper.h"
#include <QSettings>

#define SETTINGS_DOMAIN "com.canonical.TelephonyServiceHandler"

AccountProperties *AccountProperties::instance()
{
    static AccountProperties *self = new AccountProperties();
    return self;
}

QMap<QString, QVariantMap> AccountProperties::allProperties()
{
    QMap<QString,QVariantMap> props;
    for (auto accountId : TelepathyHelper::instance()->accountIds()) {
        props[accountId] = accountProperties(accountId);
    }
}

QVariantMap AccountProperties::accountProperties(const QString &accountId)
{
    QVariantMap props;
    mSettings->beginGroup(formatAccountId(accountId));
    for (auto key : mSettings->allKeys()) {
        props[key] = mSettings->value(key);
    }
    mSettings->endGroup();
    return props;
}

void AccountProperties::setAccountProperties(const QString &accountId, const QVariantMap &properties)
{
    mSettings->beginGroup(formatAccountId(accountId));
    for (auto key : properties.keys()) {
        mSettings->setValue(key, properties[key]);
    }
    mSettings->endGroup();
}

QString AccountProperties::formatAccountId(const QString &accountId)
{
    return QUrl::toPercentEncoding(accountId);
}

AccountProperties::AccountProperties(QObject *parent)
: QObject(parent),
  mSettings(new QSettings(SETTINGS_DOMAIN, QString(), parent))
{
}
