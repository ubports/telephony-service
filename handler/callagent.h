#ifndef CALLAGENT_H
#define CALLAGENT_H

#include <QObject>
#include <TelepathyQt/CallChannel>
#include <TelepathyQt/Farstream/Channel>
#include "farstreamchannel.h"

class CallAgent : public QObject
{
    Q_OBJECT
public:
    explicit CallAgent(const Tp::CallChannelPtr &channel, QObject *parent = 0);
    ~CallAgent();

protected Q_SLOTS:
    void onCallChannelInvalidated();
    void onCallStateChanged(Tp::CallState state);
    void onContentAdded(const Tp::CallContentPtr &content);
    void onStreamAdded(const Tp::CallStreamPtr &stream);

    void onFarstreamChannelCreated(Tp::PendingOperation *op);
    void onFarstreamChannelStateChanged();

private:
    Tp::CallChannelPtr mChannel;
    FarstreamChannel *mFarstreamChannel;


};

#endif // CALLAGENT_H
