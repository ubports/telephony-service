#ifndef CALLLOGPROXYMODEL_H
#define CALLLOGPROXYMODEL_H

#include <QSortFilterProxyModel>

class CallLogProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(bool onlyMissedCalls
               READ onlyMissedCalls
               WRITE setOnlyMissedCalls
               NOTIFY onlyMissedCallsChanged)
    Q_PROPERTY(QString contactId
               READ contactId
               WRITE setContactId
               NOTIFY contactIdChanged)
    Q_PROPERTY(QObject *logModel
               READ logModel
               WRITE setLogModel
               NOTIFY logModelChanged)
public:
    explicit CallLogProxyModel(QObject *parent = 0);

    bool onlyMissedCalls() const;
    QString contactId() const;
    QObject *logModel() const;

public slots:
    void setOnlyMissedCalls(bool value);
    void setContactId(QString id);
    void setLogModel(QObject *model);

signals:
    void onlyMissedCallsChanged();
    void contactIdChanged();
    void logModelChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private:
    bool mOnlyMissedCalls;
    QString mContactId;
};

#endif // CALLLOGPROXYMODEL_H
