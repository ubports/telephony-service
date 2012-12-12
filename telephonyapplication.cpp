#include "telephonyapplication.h"

#include <QDir>
#include <QUrl>
#include <QDebug>
#include <QStringList>
#include <QQuickItem>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickView>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusConnectionInterface>
#include "config.h"
#include "telephonyappdbus.h"
#include <QQmlEngine>

static void printUsage(const QStringList& arguments)
{
    qDebug() << "usage:"
             << arguments.at(0).toUtf8().constData()
             << "[contact://CONTACT_KEY]"
             << "[call://PHONE_NUMBER]"
             << "[message://PHONE_NUMBER]"
             << "[messageId://MESSAGE_ID]"
             << "[voicemail://]"
             << "[--dual-panel]"
             << "[--single-panel]"
             << "[--fullscreen]"
             << "[--test-contacts]";
}

TelephonyApplication::TelephonyApplication(int &argc, char **argv)
    : QApplication(argc, argv), m_view(0), m_applicationIsReady(false)
{
    setApplicationName("com.canonical.TelephonyApp");
    m_dbus = new TelephonyAppDBus(this);
}

bool TelephonyApplication::setup()
{
    static QList<QString> validSchemes;
    bool singlePanel = true;
    bool fullScreen = false;
    QString contactEngine = "folks";

    if (validSchemes.isEmpty()) {
        validSchemes << "contact";
        validSchemes << "call";
        validSchemes << "message";
        validSchemes << "messageId";
        validSchemes << "voicemail";
    }

    QString contactKey;
    QStringList arguments = this->arguments();

    /* Hybris gathers info on the list of running applications from the .desktop
       file specified on the command line with the desktop_file_hint switch. 
       So app will be launched like this:

       /usr/bin/launch-telephony-app --desktop_file_hint=/usr/share/applications/telephony-app.desktop

       So remove that argument and continue parsing.
    */
    for (int i = arguments.count() - 1; i >=0; --i) {
        if (arguments[i].startsWith("--desktop_file_hint")) {
            arguments.removeAt(i);
        }
    }


    if (arguments.contains("--dual-panel")) {
        arguments.removeAll("--dual-panel");
        singlePanel = false;
    }

    if (arguments.contains("--single-panel")) {
        arguments.removeAll("--single-panel");
        singlePanel = true;
    }

    if (arguments.contains("--fullscreen")) {
        arguments.removeAll("--fullscreen");
        fullScreen = true;
    }

    if (arguments.contains("--test-contacts")) {
        arguments.removeAll("--test-contacts");
        contactEngine = "memory";
    }

    if (arguments.size() > 2) {
        printUsage(arguments);
        return false;
    } else if (arguments.size() == 2) {
        QUrl uri(arguments.at(1));
        if (!validSchemes.contains(uri.scheme())) {
            printUsage(arguments);
            return false;
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

    m_view = new QQuickView();
    QObject::connect(m_view, SIGNAL(statusChanged(QQuickView::Status)), this, SLOT(onViewStatusChanged(QQuickView::Status)));
    m_view->setResizeMode(QQuickView::SizeRootObjectToView);
    m_view->setWindowTitle("Telephony");
    m_view->rootContext()->setContextProperty("application", this);
    m_view->rootContext()->setContextProperty("contactKey", contactKey);
    m_view->rootContext()->setContextProperty("dbus", m_dbus);
    m_view->rootContext()->setContextProperty("appLayout", singlePanel ? "singlePane" : "dualPane" );
    m_view->rootContext()->setContextProperty("contactEngine", contactEngine);
    m_view->engine()->setBaseUrl(QUrl::fromLocalFile(telephonyAppDirectory()));
    m_view->setSource(QUrl::fromLocalFile("telephony-app.qml"));
    if (fullScreen) {
        m_view->showFullScreen();
    } else {
        m_view->show();
    }

    connect(m_dbus,
            SIGNAL(request(QString)),
            SLOT(onMessageReceived(QString)));
    connect(m_dbus,
            SIGNAL(messageSendRequested(QString,QString)),
            SLOT(onMessageSendRequested(QString,QString)));

    return true;
}

TelephonyApplication::~TelephonyApplication()
{
    if (m_view) {
        delete m_view;
    }
}

void TelephonyApplication::onViewStatusChanged(QQuickView::Status status)
{
    if (status != QQuickView::Ready) {
        return;
    }

    QQuickItem *telephony = m_view->rootObject();
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

void TelephonyApplication::onMessageSendRequested(const QString &phoneNumber, const QString &message)
{
    QQuickItem *telephony = m_view->rootObject();
    if (!telephony) {
        return;
    }
    const QMetaObject *mo = telephony->metaObject();
    int index = mo->indexOfMethod("sendMessage(QVariant,QVariant)");
    if (index != -1) {
        QMetaMethod method = mo->method(index);
        method.invoke(telephony,
                      Q_ARG(QVariant, QVariant(phoneNumber)),
                      Q_ARG(QVariant, QVariant(message)));
    }
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

    QQuickItem *telephony = m_view->rootObject();
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
            int index = mo->indexOfMethod("startChat(QVariant)");
            if (index != -1) {
                QMetaMethod method = mo->method(index);
                method.invoke(telephony,
                              Q_ARG(QVariant, QVariant("")),
                              Q_ARG(QVariant, QVariant(value)));
            }
       }
    } else if (scheme == "messageId") {
        int index = mo->indexOfMethod("showMessage(QVariant)");
        if (index != -1) {
            QMetaMethod method = mo->method(index);
            method.invoke(telephony,
                          Q_ARG(QVariant, QVariant("")),
                          Q_ARG(QVariant, QVariant(value)));
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
        m_view->requestActivateWindow();
    } else {
        m_arg = message;
    }
}

void TelephonyApplication::activateWindow()
{
    if (m_view) {
        m_view->raise();
        m_view->requestActivateWindow();
    }
}
