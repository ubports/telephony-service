#include "components.h"
#include "telepathymanager.h"

#include <QtDebug>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/qdeclarative.h>

void Components::initializeEngine(QDeclarativeEngine *engine, const char *uri)
{
    qDebug() << "TelephonyApp initializeEngine" << uri;
    Q_ASSERT(engine);

    mRootContext = engine->rootContext();
    Q_ASSERT(mRootContext);

    mRootContext->setContextProperty(QString::fromLatin1("telepathyManager"), mTpManager);
}

void Components::registerTypes(const char *uri)
{
    qmlRegisterType<TelepathyManager>(uri, 0, 1, "TelepathyManager");
}

Q_EXPORT_PLUGIN2(components, Components);
