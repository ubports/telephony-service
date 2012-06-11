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
public:
    explicit CallLogProxyModel(QObject *parent = 0);

    bool onlyMissedCalls() const;
    void setOnlyMissedCalls(bool value);
    
signals:
    void onlyMissedCallsChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private:
    bool mOnlyMissedCalls;
};

#endif // CALLLOGPROXYMODEL_H
