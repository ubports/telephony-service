#ifndef TELEPHONYAPPLICATION_H
#define TELEPHONYAPPLICATION_H

#include <QObject>
#include <QQuickView>
#include <QApplication>

class TelephonyAppDBus;

class TelephonyApplication : public QApplication
{
    Q_OBJECT

public:
    TelephonyApplication(int &argc, char **argv);
    virtual ~TelephonyApplication();

    bool setup();

public Q_SLOTS:
    void activateWindow();

private:
    void parseArgument(const QString &arg);

private Q_SLOTS:
    void onMessageReceived(const QString &message);
    void onViewStatusChanged(QQuickView::Status status);
    void onApplicationReady();

private:
    QQuickView *m_view;
    TelephonyAppDBus *m_dbus;
    QString m_arg;
    bool m_applicationIsReady;
};

#endif // TELEPHONYAPPLICATION_H
