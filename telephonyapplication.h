#ifndef TELEPHONYAPPLICATION_H
#define TELEPHONYAPPLICATION_H

#include <QtCore/QObject>
#include <QtSingleApplication>

class QDeclarativeView;

class TelephonyApplication : public QtSingleApplication
{
    Q_OBJECT

public:
    TelephonyApplication(int &argc, char **argv);
    virtual ~TelephonyApplication();

    bool setup();

public slots:
    void onMessageReceived(const QString &message);

private:
    QDeclarativeView *m_view;
};

#endif // TELEPHONYAPPLICATION_H
