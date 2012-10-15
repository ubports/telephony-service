#ifndef CONTACTDETAIL_H
#define CONTACTDETAIL_H

#include <QString>
#include <QObject>
#include <QContactDetail>

QTCONTACTS_USE_NAMESPACE

class ContactDetail : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int type
               READ type
               NOTIFY typeChanged)
    Q_PROPERTY(QVariant contexts
               READ contexts
               WRITE setContexts
               NOTIFY detailChanged)
    Q_ENUMS(DetailType)

public:
    explicit ContactDetail(const QContactDetail &detail = QContactDetail(), QObject *parent = 0);

    enum DetailType {
        Name = QContactDetail::TypeName,
        PhoneNumber = QContactDetail::TypePhoneNumber,
        EmailAddress = QContactDetail::TypeEmailAddress,
        Address = QContactDetail::TypeAddress,
        InstantMessaging = QContactDetail::TypeOnlineAccount,
        Unknown = QContactDetail::TypeUndefined
    };

    int type() const;
    void setDetail(const QContactDetail &detail);

    QContactDetail& detail();

    /**
     * For this property and other QStringList properties that are exposed to QML, declare them as QVariant,
     * as the JavaScript array maps to Qt types as a QVariantList and not directly to QStringList.
     */
    QVariant contexts() const;
    void setContexts(const QVariant &contexts);
    
Q_SIGNALS:
    void typeChanged();
    void detailChanged();

protected:
    QContactDetail mDetail;
};

#endif // CONTACTDETAIL_H
