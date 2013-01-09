/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * This file is part of telephony-app.
 *
 * telephony-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TELEPHONYAPPLICATION_H
#define TELEPHONYAPPLICATION_H

#include <QObject>
#include <QQuickView>
#include <QGuiApplication>

class TelephonyAppDBus;

class TelephonyApplication : public QGuiApplication
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
    void onMessageSendRequested(const QString &phoneNumber, const QString &message);

private:
    QQuickView *m_view;
    TelephonyAppDBus *m_dbus;
    QString m_arg;
    bool m_applicationIsReady;
};

#endif // TELEPHONYAPPLICATION_H
