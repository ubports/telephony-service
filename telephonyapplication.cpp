#include "telephonyapplication.h"

#include <QtCore/QDir>
#include <QtCore/QUrl>
#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <QtGui/QGraphicsObject>
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
             << "[contact://CONTACT_KEY]"
             << "[call://PHONE_NUMBER]";
}

TelephonyApplication::TelephonyApplication(int &argc, char **argv)
    : QtSingleApplication(argc, argv), m_view(0), m_applicationIsReady(false)
{
}

bool TelephonyApplication::setup()
{
    static QList<QString> validSchemes;

    if (validSchemes.isEmpty()) {
        validSchemes << "contact";
        validSchemes << "call";
    }

    QString contactKey;
    QStringList arguments = this->arguments();
    if (arguments.size() > 2) {
        printUsage(arguments);
        return 1;
    } else if (arguments.size() == 2) {
        QUrl uri(arguments.at(1));
        if (!validSchemes.contains(uri.scheme())) {
            printUsage(arguments);
            return 1;
        } else {
            m_argUrl = QUrl(arguments.at(1));
        }
    }

    if (sendMessage(arguments.at(1))) {
        return false;
    }

    m_view = new QDeclarativeView();
    QObject::connect(m_view, SIGNAL(statusChanged(QDeclarativeView::Status)), this, SLOT(onViewStatusChanged));
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

void TelephonyApplication::onViewStatusChanged(QDeclarativeView::Status status)
{
    if (m_argUrl.isEmpty() || (status != QDeclarativeView::Ready)) {
        return;
    }

    QGraphicsObject *telephony = m_view->rootObject();
    if (telephony) {
        QObject::connect(telephony, SIGNAL(applicationReady()), this, SLOT(onApplicationReady()));
    }
}

void TelephonyApplication::onApplicationReady()
{
    QObject::disconnect(QObject::sender(), SIGNAL(applicationReady()), this, SLOT(onApplicationReady()));
    m_applicationIsReady = true;
    parseUrl(m_argUrl);
    m_argUrl.clear();
}

void TelephonyApplication::parseUrl(const QUrl &url)
{
    if (url.isEmpty()) {
        return;
    }

    QString scheme(url.scheme());

    if (scheme == "contact") {
        // Workaround to propagate a property change even when the contactKey was the same
        m_view->rootContext()->setContextProperty("contactKey", "");
        m_view->rootContext()->setContextProperty("contactKey", url.host());
    } else if (scheme == "call") {
        QGraphicsObject *telephony = m_view->rootObject();
        if (telephony) {
            const QMetaObject *mo = telephony->metaObject();
            int index = mo->indexOfMethod("startCallToNumber(QVariant)");
            if (index != -1) {
                QMetaMethod method = mo->method(index);
                method.invoke(telephony, Q_ARG(QVariant, QVariant(url.host())));
            }
        }
    }
}

void TelephonyApplication::onMessageReceived(const QString &message)
{
    if (m_applicationIsReady) {
        parseUrl(QUrl(message));
    } else {
        m_argUrl = QUrl(message);
    }
    activeWindow();
}
