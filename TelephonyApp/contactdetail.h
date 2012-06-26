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
    Q_PROPERTY(QStringList contexts
               READ contexts
               WRITE setContexts
               NOTIFY detailChanged)
    Q_PROPERTY(QString definitionName
               READ definitionName
               NOTIFY detailChanged)
    Q_ENUMS(DetailType)

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

    QContactDetail& detail();

    QStringList contexts() const;
    void setContexts(const QStringList &contexts);

    QString definitionName() const;
    
signals:
    void typeChanged();
    void detailChanged();

protected:
    QContactDetail mDetail;
};

#endif // CONTACTDETAIL_H
