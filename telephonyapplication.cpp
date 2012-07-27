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
#include "telephonyappdbus.h"

static void printUsage(const QStringList& arguments)
{
    qDebug() << "usage:"
             << arguments.at(0).toUtf8().constData()
             << "[contact://CONTACT_KEY]"
             << "[call://PHONE_NUMBER]"
             << "[message://PHONE_NUMBER]"
             << "[voice-email://]";
}

TelephonyApplication::TelephonyApplication(int &argc, char **argv)
    : QtSingleApplication(argc, argv), m_view(0), m_applicationIsReady(false)
{
    m_dbus = new TelephonyAppDBus(this);
}

bool TelephonyApplication::setup()
{
    static QList<QString> validSchemes;

    if (validSchemes.isEmpty()) {
        validSchemes << "contact";
        validSchemes << "call";
        validSchemes << "message";
        validSchemes << "voice-email";
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

    if (sendMessage(m_argUrl.toString())) {
        return false;
    }

    if (!m_dbus->connectToBus()) {
        qWarning() << "Failed to expose com.canonical.TelephonyApp on DBUS.";
    }

    m_view = new QDeclarativeView();
    QObject::connect(m_view, SIGNAL(statusChanged(QDeclarativeView::Status)), this, SLOT(onViewStatusChanged(QDeclarativeView::Status)));
    m_view->setResizeMode(QDeclarativeView::SizeRootObjectToView);
    m_view->setWindowTitle("Telephony");
    m_view->rootContext()->setContextProperty("contactKey", contactKey);
    m_view->rootContext()->setContextProperty("dbus", m_dbus);
    QUrl source(telephonyAppDirectory() + "/telephony-app.qml");
    m_view->setSource(source);
    m_view->show();

    setActivationWindow(m_view);

    QObject::connect(m_dbus, SIGNAL(request(QString)), this, SLOT(onMessageReceived(QString)));
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
    if (status != QDeclarativeView::Ready) {
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

    QGraphicsObject *telephony = m_view->rootObject();
    if (!telephony) {
        return;
    }
    const QMetaObject *mo = telephony->metaObject();


    QString scheme(url.scheme());
    if (scheme == "contact") {
        // Workaround to propagate a property change even when the contactKey was the same
        m_view->rootContext()->setContextProperty("contactKey", "");
        m_view->rootContext()->setContextProperty("contactKey", url.host());
    } else if (scheme == "call") {
        int index = mo->indexOfMethod("callNumber(QVariant)");
        if (index != -1) {
            QMetaMethod method = mo->method(index);
            method.invoke(telephony, Q_ARG(QVariant, QVariant(url.host())));
        }
    } else if (scheme == "message") {
        if (url.host().isEmpty()) {
            int index = mo->indexOfMethod("startNewMessage()");
            if (index != -1) {
                QMetaMethod method = mo->method(index);
                method.invoke(telephony);
            }
        } else {
            int index = mo->indexOfMethod("startChat(QVariant,QVariant,QVariant)");
            if (index != -1) {
                QMetaMethod method = mo->method(index);
                method.invoke(telephony,
                              Q_ARG(QVariant, QVariant("")),
                              Q_ARG(QVariant, QVariant(url.host())),
                              Q_ARG(QVariant, QVariant("")));
            }
       }
    } else if (scheme == "voice-email") {
        int index = mo->indexOfMethod("showVoicemail()");
        if (index != -1) {
            QMetaMethod method = mo->method(index);
            method.invoke(telephony);
        }
    }
}

void TelephonyApplication::onMessageReceived(const QString &message)
{
    if (m_applicationIsReady) {
        parseUrl(message);
        m_argUrl.clear();
    } else {
        m_argUrl = QUrl(message);
    }
    m_view->activateWindow();
    m_view->raise();
}
