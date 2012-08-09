#ifndef TELEPHONYAPPLICATION_H
#define TELEPHONYAPPLICATION_H

#include <QtCore/QObject>
#include <QtDeclarative/QDeclarativeView>
#include <QtSingleApplication>

class TelephonyAppDBus;

class TelephonyApplication : public QtSingleApplication
{
    Q_OBJECT

public:
    TelephonyApplication(int &argc, char **argv);
    virtual ~TelephonyApplication();

    bool setup();

private:
    void parseArgument(const QString &arg);

private Q_SLOTS:
    void onMessageReceived(const QString &message);
    void onViewStatusChanged(QDeclarativeView::Status status);
    void onApplicationReady();

private:
    QDeclarativeView *m_view;
    TelephonyAppDBus *m_dbus;
    QString m_arg;
    bool m_applicationIsReady;
};

#endif // TELEPHONYAPPLICATION_H
