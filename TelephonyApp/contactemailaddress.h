#ifndef CONTACTEMAILADDRESS_H
#define CONTACTEMAILADDRESS_H

#include "contactdetail.h"

class ContactEmailAddress : public ContactDetail
{
    Q_OBJECT
    Q_PROPERTY(QString emailAddress
               READ emailAddress
               WRITE setEmailAddress
               NOTIFY changed)
public:
    explicit ContactEmailAddress(const QContactDetail &detail, QObject *parent = 0);
    
    int type() const;

    QString emailAddress() const;
    void setEmailAddress(const QString &value);

signals:
    void changed();
    
};

#endif // CONTACTEMAILADDRESS_H
