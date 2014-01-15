#include "handlercontroller.h"

#define HANDLER_SERVICE "com.canonical.TelephonyServiceHandler"
#define HANDLER_OBJECT "/com/canonical/TelephonyServiceHandler"
#define HANDLER_INTERFACE "com.canonical.TelephonyServiceHandler"

HandlerController *HandlerController::instance()
{
    static HandlerController *self = new HandlerController();
    return self;
}

HandlerController::HandlerController(QObject *parent) :
    QObject(parent),
    mHandlerInterface(HANDLER_SERVICE, HANDLER_OBJECT, HANDLER_INTERFACE)
{
}

void HandlerController::startCall(const QString &number)
{
    mHandlerInterface.call("StartCall", number);
}

void HandlerController::hangUpCall(const QString &objectPath)
{
    mHandlerInterface.call("HangUpCall", objectPath);
}

void HandlerController::setHold(const QString &objectPath, bool hold)
{
    mHandlerInterface.call("SetHold", objectPath, hold);
}

void HandlerController::setMuted(const QString &objectPath, bool muted)
{
    mHandlerInterface.call("SetMuted", objectPath, muted);
}

void HandlerController::setSpeakerMode(const QString &objectPath, bool enabled)
{
    mHandlerInterface.call("SetSpeakerMode", objectPath, enabled);
}

void HandlerController::sendDTMF(const QString &objectPath, const QString &key)
{
    mHandlerInterface.call("SendDTMF", objectPath, key);
}

void HandlerController::sendMessage(const QString &number, const QString &message)
{
    mHandlerInterface.call("SendMessage", number, message);
}

void HandlerController::acknowledgeMessages(const QString &number, const QStringList &messageIds)
{
    mHandlerInterface.call("AcknowledgeMessages", number, messageIds);
}
