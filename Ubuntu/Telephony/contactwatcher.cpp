#include "contactwatcher.h"
#include <QContactManager>
#include <QContactFetchByIdRequest>
#include <QContactFetchRequest>
#include <QContactAvatar>
#include <QContactDisplayLabel>
#include <QContactDetailFilter>

QContactManager *ContactWatcher::engineInstance()
{
    static QContactManager* manager = new QContactManager("galera");
    return manager;
}

ContactWatcher::ContactWatcher(QObject *parent) :
    QObject(parent),
    mIsUnknown(true)
{
    mContactManager = engineInstance();
    connect(mContactManager,
            SIGNAL(contactsAdded(QList<QContactId>)),
            SLOT(onContactsAdded(QList<QContactId>)));
    connect(mContactManager,
            SIGNAL(contactsChanged(QList<QContactId>)),
            SLOT(onContactsChanged(QList<QContactId>)));
    connect(mContactManager,
            SIGNAL(contactsRemoved(QList<QContactId>)),
            SLOT(onContactsRemoved(QList<QContactId>)));
}

void ContactWatcher::setId(const QString &id)
{
    qDebug() << "setId" << id;
    mId = id;
    QContactFetchByIdRequest *request = new QContactFetchByIdRequest(this);
    QList<QContactId> ids;
    ids << QContactId::fromString(id);
    request->setIds(ids);
    connect(request, SIGNAL(stateChanged(QContactAbstractRequest::State)), SLOT(onRequestStateChanged(QContactAbstractRequest::State)));
    request->setManager(mContactManager);
    request->start();
}

QString ContactWatcher::id() const
{
    return mId;
}

QString ContactWatcher::avatar() const
{
    return mAvatar;
}

QString ContactWatcher::alias() const
{
    return mAlias;
}

QString ContactWatcher::phoneNumber() const
{
    return mPhoneNumber;
}

void ContactWatcher::setPhoneNumber(const QString &phoneNumber)
{
    qDebug() << "setPhoneNumber" << phoneNumber;
    if (phoneNumber.isEmpty())
        return;
    mPhoneNumber = phoneNumber;

    QContactFetchRequest *request = new QContactFetchRequest(this);
    QContactDetailFilter filter;
    filter.setMatchFlags(QContactFilter::MatchPhoneNumber);
    filter.setDetailType(QContactDetail::TypePhoneNumber);
    filter.setValue(phoneNumber);
    request->setFilter(filter);
    connect(request, SIGNAL(stateChanged(QContactAbstractRequest::State)), SLOT(onRequestStateChanged(QContactAbstractRequest::State)));
    connect(request, SIGNAL(resultsAvailable()), SLOT(resultsAvailable()));
    request->setManager(mContactManager);
    request->start();
}

bool ContactWatcher::isUnknown() const
{
    return mIsUnknown;
}


void ContactWatcher::onContactsAdded(QList<QContactId> ids)
{

}

void ContactWatcher::onContactsChanged(QList<QContactId> ids)
{

}

void ContactWatcher::onContactsRemoved(QList<QContactId> ids)
{

}
void ContactWatcher::resultsAvailable()
{
    qDebug() << "resultsAvailable";
}

void ContactWatcher::onRequestStateChanged(QContactAbstractRequest::State state)
{
    qDebug() << "requestChanged";
    QContactFetchRequest *request = qobject_cast<QContactFetchRequest*>(sender());
    if (state == QContactAbstractRequest::FinishedState) {
        if (request->contacts().size() > 0) {
            QContact contact = request->contacts().at(0);
            mId = contact.id().toString();
            mAvatar = QContactAvatar(contact.detail(QContactDetail::TypeAvatar)).imageUrl().toString();
            mAlias = QContactDisplayLabel(contact.detail(QContactDetail::TypeDisplayLabel)).label();
            qDebug() << mId << mAvatar << mAlias;
            Q_EMIT idChanged();
            Q_EMIT avatarChanged();
            Q_EMIT aliasChanged();
        }
        request->deleteLater();
    }
}
