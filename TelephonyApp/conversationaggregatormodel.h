#ifndef CONVERSATIONAGGREGATORMODEL_H
#define CONVERSATIONAGGREGATORMODEL_H

#include <QAbstractListModel>

class ConversationFeedModel;

class ConversationAggregatorModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ConversationAggregatorModel(QObject *parent = 0);

    void addFeedModel(ConversationFeedModel *model);
    void removeFeedModel(ConversationFeedModel *model);

    QVariant data(const QModelIndex &index, int role) const;
    int rowCount(const QModelIndex &parent) const;

    QModelIndex mapFromSource(const QModelIndex &index) const;
    QModelIndex mapToSource(const QModelIndex &index) const;

private Q_SLOTS:
    void updateOffsets();
    void onRowsInserted(const QModelIndex &parent, int start, int end);
    void onRowsRemoved(const QModelIndex &parent, int start, int end);
    void onModelReset();

private:
    QList<ConversationFeedModel*> mFeedModels;
    QMap<const ConversationFeedModel*,int> mModelOffsets;
    int mRowCount;
};

#endif // CONVERSATIONAGGREGATORMODEL_H
