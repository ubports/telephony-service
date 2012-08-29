#include "telephonyapplication.h"

#include <QtCore/QDir>
#include <QtCore/QUrl>
#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <QtGui/QGraphicsObject>
#include <QtDeclarative/QDeclarativeComponent>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeView>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusConnectionInterface>
#include "config.h"
#include "telephonyappdbus.h"

static void printUsage(const QStringList& arguments)
{
    qDebug() << "usage:"
             << arguments.at(0).toUtf8().constData()
             << "[contact://CONTACT_KEY]"
             << "[call://PHONE_NUMBER]"
             << "[message://PHONE_NUMBER]"
             << "[voicemail://]";
}

TelephonyApplication::TelephonyApplication(int &argc, char **argv)
    : QApplication(argc, argv), m_view(0), m_applicationIsReady(false)
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
        validSchemes << "voicemail";
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
            m_arg = arguments.at(1);
        }
    }

    // check if the app is already running, if it is, send the message to the running instance
    QDBusReply<bool> reply = QDBusConnection::sessionBus().interface()->isServiceRegistered("com.canonical.TelephonyApp");
    if (reply.isValid() && reply.value()) {
        QDBusInterface appInterface("com.canonical.TelephonyApp",
                                    "/com/canonical/TelephonyApp",
                                    "com.canonical.TelephonyApp");
        appInterface.call("SendAppMessage", m_arg);
        return false;
    }

    if (!m_dbus->connectToBus()) {
        qWarning() << "Failed to expose com.canonical.TelephonyApp on DBUS.";
    }

    m_view = new QDeclarativeView();
    QObject::connect(m_view, SIGNAL(statusChanged(QDeclarativeView::Status)), this, SLOT(onViewStatusChanged(QDeclarativeView::Status)));
    m_view->setResizeMode(QDeclarativeView::SizeRootObjectToView);
    m_view->setWindowTitle("Telephony");
    m_view->rootContext()->setContextProperty("application", this);
    m_view->rootContext()->setContextProperty("contactKey", contactKey);
    m_view->rootContext()->setContextProperty("dbus", m_dbus);
    QUrl source(telephonyAppDirectory() + "/telephony-app.qml");
    m_view->setSource(source);
    m_view->show();

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
    parseArgument(m_arg);
    m_arg.clear();
}

void TelephonyApplication::parseArgument(const QString &arg)
{
    if (arg.isEmpty()) {
        return;
    }

    QStringList args = arg.split("://");
    if (args.size() != 2) {
        return;
    }

    QString scheme = args[0];
    QString value = args[1];

    QGraphicsObject *telephony = m_view->rootObject();
    if (!telephony) {
        return;
    }
    const QMetaObject *mo = telephony->metaObject();


    if (scheme == "contact") {
        // Workaround to propagate a property change even when the contactKey was the same
        m_view->rootContext()->setContextProperty("contactKey", "");
        m_view->rootContext()->setContextProperty("contactKey", value);
    } else if (scheme == "call") {
        int index = mo->indexOfMethod("callNumber(QVariant)");
        if (index != -1) {
            QMetaMethod method = mo->method(index);
            method.invoke(telephony, Q_ARG(QVariant, QVariant(value)));
        }
    } else if (scheme == "message") {
        if (value.isEmpty()) {
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
                              Q_ARG(QVariant, QVariant(value)),
                              Q_ARG(QVariant, QVariant("")));
            }
       }
    } else if (scheme == "voicemail") {
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
        parseArgument(message);
        m_arg.clear();
        m_view->activateWindow();
    } else {
        m_arg = message;
    }
}
