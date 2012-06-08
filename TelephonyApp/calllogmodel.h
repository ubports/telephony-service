#ifndef CALLLOGMODEL_H
#define CALLLOGMODEL_H

#include <QAbstractListModel>
#include <TelepathyLoggerQt4/PendingOperation>
#include <QDateTime>
#include <QList>
#include <QUrl>

class CallEntry {
public:
    QString contactId;
    QString contactAlias;
    QUrl avatar;
    QString phoneNumber;
    QString phoneType;
    QDateTime timestamp;
    bool missed;
    bool incoming;
};

namespace QtMobility {
    class QContactManager;
}

class CallLogModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum CallLogRoles {
        ContactId = Qt::UserRole,
        ContactAlias,
        Avatar,
        PhoneNumber,
        PhoneType,
        Timestamp,
        Missed,
        Incoming
    };

    explicit CallLogModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    
protected:
    void fetchCallLog();

protected slots:
    void onPendingEntitiesFinished(Tpl::PendingOperation *op);
    void onPendingDatesFinished(Tpl::PendingOperation *op);
    void onPendingEventsFinished(Tpl::PendingOperation *op);

private:
    QList<CallEntry> mCallEntries;
    QtMobility::QContactManager *mContactManager;

};

#endif // CALLLOGMODEL_H
