/*
 * Copyright (C) 2012-2013 Canonical, Ltd.
 *
 * This file is part of phone-app.
 *
 * phone-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * phone-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PHONEAPPLICATION_H
#define PHONEAPPLICATION_H

#include <QObject>
#include <QQuickView>
#include <QGuiApplication>

class PhoneAppDBus;

class PhoneApplication : public QGuiApplication
{
    Q_OBJECT

public:
    PhoneApplication(int &argc, char **argv);
    virtual ~PhoneApplication();

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
    PhoneAppDBus *m_dbus;
    QString m_arg;
    bool m_applicationIsReady;
};

#endif // PHONEAPPLICATION_H
