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
    Q_PROPERTY(QVariant contexts
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

    /**
     * For this property and other QStringList properties that are exposed to QML, declare them as QVariant,
     * as the JavaScript array maps to Qt types as a QVariantList and not directly to QStringList.
     */
    QVariant contexts() const;
    void setContexts(const QVariant &contexts);

    QString definitionName() const;
    
Q_SIGNALS:
    void typeChanged();
    void detailChanged();

protected:
    QContactDetail mDetail;
};

#endif // CONTACTDETAIL_H
