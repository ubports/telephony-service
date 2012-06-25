#ifndef CONTACTDETAIL_H
#define CONTACTDETAIL_H

#include <QObject>
#include <QContactDetail>

using namespace QtMobility;

class ContactDetail : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int type
               READ type
               NOTIFY typeChanged)
public:
    explicit ContactDetail(const QContactDetail &detail = QContactDetail(), QObject *parent = 0);

    enum DetailType {
        Name,
        PhoneNumber,
        EmailAddress,
        Address,
        InstantMessaging,
        Unknown
    };

    virtual int type() const;
    void setDetail(const QContactDetail &detail);
    
signals:
    void typeChanged();
    void detailChanged();

protected:
    QContactDetail mDetail;
};

#endif // CONTACTDETAIL_H
