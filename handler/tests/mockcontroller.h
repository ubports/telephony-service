/**
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 */

#ifndef MOCKCONTROLLER_H
#define MOCKCONTROLLER_H

#include <QObject>
#include <QDBusInterface>

class MockController : public QObject
{
    Q_OBJECT
public:
    static MockController *instance();

Q_SIGNALS:
    void messageSent(const QString &message, const QVariantMap &properties);
    void callReceived(const QString &callerId);
    void callEnded(const QString &callerId);
    void callStateChanged(const QString &callerId, const QString &objectPath, const QString &state);

public Q_SLOTS:
    void placeIncomingMessage(const QString &message, const QVariantMap &properties);
    QString placeCall(const QVariantMap &properties);
    void hangupCall(const QString &callerId);
    void setCallState(const QString &phoneNumber, const QString &state);

private:
    explicit MockController(QObject *parent = 0);
    QDBusInterface mMockInterface;
};

#endif // MOCKCONTROLLER_H
