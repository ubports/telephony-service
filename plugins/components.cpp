#include "components.h"

#include <QtDebug>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/qdeclarative.h>

void Components::initializeEngine(QDeclarativeEngine *engine, const char *uri)
{
    qDebug() << "TelephonyApp initializeEngine" << uri;
    Q_ASSERT(engine);
}

void Components::registerTypes(const char *uri)
{
}

Q_EXPORT_PLUGIN2(components, Components);
