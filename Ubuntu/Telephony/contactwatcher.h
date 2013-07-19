#ifndef CONTACTWATCHER_H
#define CONTACTWATCHER_H

#include <QObject>
#include <QContactManager>
#include <QContactAbstractRequest>

QTCONTACTS_USE_NAMESPACE

class ContactWatcher : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString contactId READ contactId WRITE setContactId NOTIFY contactIdChanged)
    Q_PROPERTY(QString avatar READ avatar NOTIFY avatarChanged)
    Q_PROPERTY(QString alias READ alias NOTIFY aliasChanged)
    Q_PROPERTY(QString phoneNumber READ phoneNumber WRITE setPhoneNumber NOTIFY phoneNumberChanged)
    Q_PROPERTY(bool isUnknown READ isUnknown NOTIFY isUnknownChanged)
public:
    explicit ContactWatcher(QObject *parent = 0);

    static QContactManager *engineInstance();

    QString contactId() const;
    void setContactId(const QString &contactId);
    QString avatar() const;
    QString alias() const;
    QString phoneNumber() const;
    void setPhoneNumber(const QString &phoneNumber);
    bool isUnknown() const;
    
Q_SIGNALS:
    void contactIdChanged();
    void avatarChanged();
    void aliasChanged();
    void phoneNumberChanged();
    void isUnknownChanged();

protected Q_SLOTS:
    void onContactsAdded(QList<QContactId> ids);
    void onContactsChanged(QList<QContactId> ids);
    void onContactsRemoved(QList<QContactId> ids);
    void onRequestStateChanged(QContactAbstractRequest::State state);
    void resultsAvailable();

private:
    QString mContactId;
    QString mAvatar;
    QString mAlias;
    QString mPhoneNumber;
    bool mIsUnknown;
    QContactManager *mContactManager;
};

#endif // CONTACTWATCHER_H
