#ifndef TELEPHONYAPPLICATION_H
#define TELEPHONYAPPLICATION_H

#include <QtCore/QObject>
#include <QtCore/QUrl>
#include <QtDeclarative/QDeclarativeView>
#include <QtSingleApplication>

class TelephonyApplication : public QtSingleApplication
{
    Q_OBJECT

public:
    TelephonyApplication(int &argc, char **argv);
    virtual ~TelephonyApplication();

    bool setup();

private:
    void parseUrl(const QUrl &url);

private slots:
    void onMessageReceived(const QString &message);
    void onViewStatusChanged(QDeclarativeView::Status status);
    void onApplicationReady();

private:
    QDeclarativeView *m_view;
    QUrl m_argUrl;
    bool m_applicationIsReady;
};

#endif // TELEPHONYAPPLICATION_H
