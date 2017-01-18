#ifndef ACCOUNTPROPERTIES_H
#define ACCOUNTPROPERTIES_H

#include <QObject>

class QSettings;

class AccountProperties : public QObject
{
    Q_OBJECT
public:
    static AccountProperties *instance();

    QMap<QString,QVariantMap> allProperties();
    QVariantMap accountProperties(const QString &accountId);
    void setAccountProperties(const QString &accountId, const QVariantMap &properties);

protected:
    explicit AccountProperties(QObject *parent = 0);

private:
    QSettings *mSettings;
};

#endif // ACCOUNTPROPERTIES_H
