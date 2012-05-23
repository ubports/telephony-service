#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "channelhandler.h"

#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeExtensionPlugin>

class ChannelHandler;

class Components : public QDeclarativeExtensionPlugin
{
    Q_OBJECT

public:
    void initializeEngine(QDeclarativeEngine *engine, const char *uri);
    void registerTypes(const char *uri);

private Q_SLOTS:
    void onChannelHandlerCreated(ChannelHandler *ch);

private:
    QDeclarativeContext *mRootContext;
};

#endif // COMPONENTS_H
