#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeExtensionPlugin>

class Components : public QDeclarativeExtensionPlugin
{
    Q_OBJECT

public:
    void initializeEngine(QDeclarativeEngine *engine, const char *uri);
    void registerTypes(const char *uri);
};

#endif // COMPONENTS_H
