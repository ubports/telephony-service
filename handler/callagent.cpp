#include "callagent.h"
#include <TelepathyQt/CallContent>
#include <TelepathyQt/Contact>
#include <TelepathyQt/Farstream/Channel>
#include <QDebug>

CallAgent::CallAgent(const Tp::CallChannelPtr &channel, QObject *parent) :
    QObject(parent), mChannel(channel), mFarstreamChannel(0)
{
    connect(mChannel.data(),
            SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
            SLOT(onCallChannelInvalidated()));
    connect(mChannel.data(),
            SIGNAL(callStateChanged(Tp::CallState)),
            SLOT(onCallStateChanged(Tp::CallState)));
    connect(mChannel.data(),
            SIGNAL(contentAdded(Tp::CallContentPtr)),
            SLOT(onContentAdded(Tp::CallContentPtr)));

    Q_FOREACH(const Tp::CallContentPtr &content, mChannel->contents()) {
        onContentAdded(content);
    }

    if (!mChannel->handlerStreamingRequired()) {
        return;
    }

    Tp::Farstream::PendingChannel *pendingChannel = Tp::Farstream::createChannel(mChannel);
    connect(pendingChannel,
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onFarstreamChannelCreated(Tp::PendingOperation*)));
}

CallAgent::~CallAgent()
{
    if (mFarstreamChannel) {
        mFarstreamChannel->deleteLater();
    }
}

void CallAgent::onCallChannelInvalidated()
{
    deleteLater();
}

void CallAgent::onCallStateChanged(Tp::CallState state)
{

}

void CallAgent::onContentAdded(const Tp::CallContentPtr &content)
{
    if (!mChannel->handlerStreamingRequired()) {
        return;
    }

    qDebug() << "Content Added, name: " << content->name() << " type: " << content->type();

    connect(content.data(),
            SIGNAL(streamAdded(Tp::CallStreamPtr)),
            SLOT(onStreamAdded(Tp::CallStreamPtr)));

    Q_FOREACH(const Tp::CallStreamPtr &stream, content->streams()) {
        onStreamAdded(stream);
    }
}

void CallAgent::onStreamAdded(const Tp::CallStreamPtr &stream)
{
    qDebug() << "Stream present: " << stream->localSendingState();

    qDebug() << "  members " << stream->remoteMembers().size();
    Q_FOREACH(const Tp::ContactPtr contact, stream->remoteMembers()) {
        qDebug() << "    member " << contact->id() << " remoteSendingState=" << stream->remoteSendingState(contact);
    }
}

void CallAgent::onFarstreamChannelCreated(Tp::PendingOperation *op)
{
    Tp::Farstream::PendingChannel *pendingChannel = qobject_cast<Tp::Farstream::PendingChannel*>(op);
    if (!pendingChannel) {
        return;
    }

    mFarstreamChannel = new FarstreamChannel(pendingChannel->tfChannel());

    connect(mFarstreamChannel,
            SIGNAL(stateChanged()),
            SLOT(onFarstreamChannelStateChanged()));

    mFarstreamChannel->init();
}

void CallAgent::onFarstreamChannelStateChanged()
{
    qDebug() << "CallAgent::onFarstreamStateChanged: mFarstreamChannel=" << mFarstreamChannel;

    if (!mFarstreamChannel) {
        return;
    }

    if (mFarstreamChannel->state() == Tp::MediaStreamStateConnected) {
        qDebug() << "BLABLA set mute and volume";
    }
}
