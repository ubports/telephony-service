#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "telepathymanager.h"
#include "channelhandler.h"
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeExtensionPlugin>

class Components : public QDeclarativeExtensionPlugin
{
    Q_OBJECT

public:
    void initializeEngine(QDeclarativeEngine *engine, const char *uri);
    void registerTypes(const char *uri);

private:
    QDeclarativeContext *mRootContext;
    TelepathyManager *mTpManager;
    ChannelHandler *mChannelHandler;

};

#endif // COMPONENTS_H
