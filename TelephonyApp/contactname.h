#ifndef CONTACTNAME_H
#define CONTACTNAME_H

#include "contactdetail.h"
#include <QContactName>

class ContactName : public ContactDetail
{
    Q_OBJECT
    Q_PROPERTY(QString customLabel READ customLabel WRITE setCustomLabel NOTIFY changed)
    Q_PROPERTY(QString firstName READ firstName WRITE setFirstName NOTIFY changed)
    Q_PROPERTY(QString lastName READ lastName WRITE setLastName NOTIFY changed)
    Q_PROPERTY(QString middleName READ middleName WRITE setMiddleName NOTIFY changed)
    Q_PROPERTY(QString prefix READ prefix WRITE setPrefix NOTIFY changed)
    Q_PROPERTY(QString suffix READ suffix WRITE setSuffix NOTIFY changed)

public:
    explicit ContactName(const QContactDetail &detail = QContactName(), QObject *parent = 0);

    int type() const;

    QString customLabel() const;
    void setCustomLabel(const QString &value);

    QString firstName() const;
    void setFirstName(const QString &value);

    QString lastName() const;
    void setLastName(const QString &value);

    QString middleName () const;
    void setMiddleName(const QString &value);

    QString prefix () const;
    void setPrefix(const QString &value);

    QString suffix() const;
    void setSuffix(const QString &value);

Q_SIGNALS:
    void changed();
};

#endif // CONTACTNAME_H
