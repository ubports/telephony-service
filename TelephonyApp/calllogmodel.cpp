#include "calllogmodel.h"
#include "telepathyhelper.h"
#include <TelepathyLoggerQt4/LogManager>
#include <TelepathyLoggerQt4/PendingDates>
#include <TelepathyLoggerQt4/PendingEntities>
#include <TelepathyLoggerQt4/PendingEvents>
#include <TelepathyLoggerQt4/Entity>
#include <TelepathyLoggerQt4/Event>
#include <TelepathyLoggerQt4/CallEvent>
#include <QContactManager>
#include <QContactDetailFilter>
#include <QContactAvatar>
#include <QContactUrl>

CallLogModel::CallLogModel(QObject *parent) :
    QAbstractListModel(parent)
{
    qDebug() << "Call log model created";
    mContactManager = new QtMobility::QContactManager("folks", QMap<QString,QString>(), this);
    // set the role names
    QHash<int, QByteArray> roles;
    roles[ContactId] = "contactId";
    roles[ContactAlias] = "contactAlias";
    roles[Avatar] = "avatar";
    roles[PhoneNumber] = "phoneNumber";
    roles[PhoneType] = "phoneType";
    roles[Timestamp] = "timestamp";
    roles[Missed] = "missed";
    roles[Incoming] = "incoming";
    setRoleNames(roles);

    fetchCallLog();
}

int CallLogModel::rowCount(const QModelIndex &parent) const
{
    // no child items
    if (parent.isValid()) {
        return 0;
    }

    return mCallEntries.count();
}

QVariant CallLogModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= mCallEntries.count()) {
        return QVariant();
    }

    switch (role) {
    case ContactId:
        return mCallEntries[index.row()].contactId;
    case ContactAlias:
    case Qt::DisplayRole:
        return mCallEntries[index.row()].contactAlias;
    case Avatar:
    case Qt::DecorationRole:
        return mCallEntries[index.row()].avatar;
    case PhoneNumber:
        return mCallEntries[index.row()].phoneNumber;
    case PhoneType:
        return mCallEntries[index.row()].phoneType;
    case Timestamp:
        return mCallEntries[index.row()].timestamp;
    case Missed:
        return mCallEntries[index.row()].missed;
    case Incoming:
        return mCallEntries[index.row()].incoming;
    default:
        return QVariant();
    }
}

void CallLogModel::fetchCallLog()
{
    Tpl::LogManagerPtr manager = Tpl::LogManager::instance();
    Tpl::PendingEntities *pendingEntities = manager->queryEntities(TelepathyHelper::instance()->account());

    /* Fetching the call log work like this:
       - Start by fetching the entities from the log
       - Once you get the entities, fetch the available dates
       - After you get the dates, fetch the events themselves
     */
    connect(pendingEntities,
            SIGNAL(finished(Tpl::PendingOperation*)),
            SLOT(onPendingEntitiesFinished(Tpl::PendingOperation*)));
}

void CallLogModel::onPendingEntitiesFinished(Tpl::PendingOperation *op)
{
    Tpl::PendingEntities *pe = qobject_cast<Tpl::PendingEntities*>(op);
    if (!pe) {
        return;
    }

    Tpl::EntityPtrList entities = pe->entities();
    Tpl::LogManagerPtr manager = Tpl::LogManager::instance();
    Tp::AccountPtr account = TelepathyHelper::instance()->account();

    foreach(Tpl::EntityPtr entity, entities) {
        Tpl::PendingDates *pendingDates = manager->queryDates(account, entity, Tpl::EventTypeMaskAny);

        connect(pendingDates,
                SIGNAL(finished(Tpl::PendingOperation*)),
                SLOT(onPendingDatesFinished(Tpl::PendingOperation*)));
    }
}

void CallLogModel::onPendingDatesFinished(Tpl::PendingOperation *op)
{
    Tpl::PendingDates *pd = qobject_cast<Tpl::PendingDates*>(op);
    if (!pd) {
        return;
    }

    Tpl::QDateList dates = pd->dates();
    Tpl::LogManagerPtr manager = Tpl::LogManager::instance();
    Tp::AccountPtr account = TelepathyHelper::instance()->account();

    foreach(QDate date, dates) {
        Tpl::PendingEvents *pendingEvents = manager->queryEvents(account, pd->entity(), Tpl::EventTypeMaskAny, date);
        connect(pendingEvents,
                SIGNAL(finished(Tpl::PendingOperation*)),
                SLOT(onPendingEventsFinished(Tpl::PendingOperation*)));
    }
}

void CallLogModel::onPendingEventsFinished(Tpl::PendingOperation *op)
{
    Tpl::PendingEvents *pe = qobject_cast<Tpl::PendingEvents*>(op);
    if (!pe) {
        return;
    }

    Tpl::EventPtrList events = pe->events();
    Tp::AccountPtr account = TelepathyHelper::instance()->account();

    QtMobility::QContactDetailFilter filter;
    filter.setDetailDefinitionName("PhoneNumber", "phoneNumber");

    // add the events to the list
    foreach(Tpl::EventPtr event, events) {
        Tpl::CallEventPtr callEvent = event.dynamicCast<Tpl::CallEvent>();
        if (callEvent) {
            CallEntry entry;
            entry.incoming = (callEvent->receiver()->entityType() == Tpl::EntityTypeSelf);
            entry.timestamp = callEvent->timestamp();
            entry.missed = (callEvent->endReason() == Tp::CallStateChangeReasonNoAnswer);

            Tpl::EntityPtr remoteEntity = entry.incoming ? callEvent->sender() : callEvent->receiver();
            entry.contactAlias = remoteEntity->alias();
            entry.phoneNumber = remoteEntity->identifier();

            // fetch the QContact object
            filter.setValue(remoteEntity->identifier());
            QList<QtMobility::QContact> contacts = mContactManager->contacts(filter);
            if (contacts.count() > 0) {
                // if more than one contact matches, use the first one
                QtMobility::QContactUrl url = contacts[0].detail<QtMobility::QContactUrl>();
                QtMobility::QContactAvatar avatar = contacts[0].detail<QtMobility::QContactAvatar>();
                entry.contactId = url.url();
                entry.avatar = avatar.imageUrl();
            }

            beginInsertRows(QModelIndex(), mCallEntries.count(), mCallEntries.count());
            mCallEntries.append(entry);
            endInsertRows();
        }
    }
}
