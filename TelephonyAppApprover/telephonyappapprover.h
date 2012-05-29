#ifndef TELEPATHYAPPAPPROVER_H
#define TELEPATHYAPPAPPROVER_H

#include <QMap>
#include <TelepathyQt4/AbstractClientApprover>
#include <TelepathyQt4/PendingReady>

class TelephonyAppApprover : public QObject, public Tp::AbstractClientApprover
{
    Q_OBJECT
public:
    TelephonyAppApprover();
    ~TelephonyAppApprover();

    void addDispatchOperation(const Tp::MethodInvocationContextPtr<> &context,
                              const Tp::ChannelDispatchOperationPtr &dispatchOperation);
private Q_SLOTS:
    void onChannelReady(Tp::PendingOperation *op);
private:
    QList<Tp::ChannelDispatchOperationPtr> mDispatchOps;
    QMap<Tp::PendingReady*,Tp::ChannelPtr> mChannels;
};

#endif // TELEPATHYAPPAPPROVER_H
