#include "calllogproxymodel.h"
#include "calllogmodel.h"

CallLogProxyModel::CallLogProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent), mOnlyMissedCalls(false)
{
}

bool CallLogProxyModel::onlyMissedCalls() const
{
    return mOnlyMissedCalls;
}

void CallLogProxyModel::setOnlyMissedCalls(bool value)
{
    if (value != mOnlyMissedCalls) {
        mOnlyMissedCalls = value;
        emit onlyMissedCallsChanged();
    }
}

bool CallLogProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (!sourceModel()) {
        return false;
    }

    // if we are not filtering to show only missed calls, just return true
    if (!mOnlyMissedCalls) {
        return true;
    }

    QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);
    return sourceIndex.data(CallLogModel::Missed).toBool();
}
