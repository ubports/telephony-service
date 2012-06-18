#include "telephonyapplication.h"

#include <QtCore/QDir>
#include <QtCore/QUrl>
#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <QtDeclarative/QDeclarativeComponent>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeView>

#include "config.h"

// inspired by qmlviewer’s QDeclarativeViewer::loadDummyDataFiles(…)
static void loadDummyDataFiles(QDeclarativeView* view)
{
    QDir dir(telephonyAppDirectory() + "/dummydata", "*.qml");
    Q_FOREACH(const QString& qmlFile, dir.entryList()) {
        const QString filePath = dir.filePath(qmlFile);
        QDeclarativeComponent comp(view->engine(), filePath);
        QObject* dummyData = comp.create();
        if(comp.isError()) {
            Q_FOREACH(const QDeclarativeError &error, comp.errors()) {
                qWarning() << error;
            }
        }
        if (dummyData) {
            qDebug() << "Loaded dummy data:" << filePath;
            QString propertyName = qmlFile;
            propertyName.chop(4);
            view->rootContext()->setContextProperty(propertyName, dummyData);
            dummyData->setParent(view);
        }
    }
}

static void printUsage(const QStringList& arguments)
{
    qDebug() << "usage:"
             << arguments.at(0).toUtf8().constData()
             << "[contact://CONTACT_KEY]";
}

TelephonyApplication::TelephonyApplication(int &argc, char **argv)
    : QtSingleApplication(argc, argv), m_view(0)
{
}

bool TelephonyApplication::setup()
{
    QString contactKey;
    QStringList arguments = this->arguments();
    if (arguments.size() > 2) {
        printUsage(arguments);
        return 1;
    } else if (arguments.size() == 2) {
        QString contactUri = arguments.at(1);
        QString contactUriScheme = "contact://";
        if (!contactUri.startsWith(contactUriScheme)) {
            printUsage(arguments);
            return 1;
        } else {
            contactKey = contactUri.mid(contactUriScheme.size());
        }
    }

    if (sendMessage(contactKey)) {
        return false;
    }

    m_view = new QDeclarativeView();
    m_view->setResizeMode(QDeclarativeView::SizeRootObjectToView);
    loadDummyDataFiles(m_view);
    m_view->rootContext()->setContextProperty("contactKey", contactKey);
    QUrl source(telephonyAppDirectory() + "/telephony-app.qml");
    m_view->setSource(source);
    m_view->show();

    setActivationWindow(m_view);

    QObject::connect(this, SIGNAL(messageReceived(QString)), this, SLOT(onMessageReceived(QString)));
    return true;
}

TelephonyApplication::~TelephonyApplication()
{
    if (m_view) {
        delete m_view;
    }
}

void TelephonyApplication::onMessageReceived(const QString &message)
{
    if (m_view) {
        if (message.length()) {
            // Workaround to propagate a property change even when the contactKey was the same
            m_view->rootContext()->setContextProperty("contactKey", "");
            m_view->rootContext()->setContextProperty("contactKey", message);
        }
        activeWindow();
    }
}
