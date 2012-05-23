#include "components.h"
#include "telepathyhelper.h"
#include "channelhandler.h"

#include <QtDebug>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/qdeclarative.h>
#include <TelepathyQt/Debug>

void Components::initializeEngine(QDeclarativeEngine *engine, const char *uri)
{
    qDebug() << "TelephonyApp initializeEngine" << uri;
    Q_ASSERT(engine);

    // Initialize telepathy types
    Tp::registerTypes();
    //Tp::enableDebug(true);
    Tp::enableWarnings(true);

    mRootContext = engine->rootContext();
    Q_ASSERT(mRootContext);

    mRootContext->setContextProperty("telepathyManager", TelepathyHelper::instance());
    mRootContext->setContextProperty("chatManager", TelepathyHelper::instance()->chatManager());

    connect(TelepathyHelper::instance(),
            SIGNAL(channelHandlerCreated(ChannelHandler*)),
            SLOT(onChannelHandlerCreated(ChannelHandler*)));
}

void Components::registerTypes(const char *uri)
{
    // @uri TelephonyApp
    qmlRegisterType<TelepathyHelper>(uri, 0, 1, "TelepathyHelper");
}

void Components::onChannelHandlerCreated(ChannelHandler *ch)
{
    // register the context property
    mRootContext->setContextProperty("channelHandler", ch);
    mRootContext->setContextProperty("telepathyManager", ch);
}

Q_EXPORT_PLUGIN2(components, Components)
