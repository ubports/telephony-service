/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MESSAGELOGMODEL_H
#define MESSAGELOGMODEL_H

#include "abstractloggermodel.h"

class MessageLogEntry : public LogEntry {
public:
    QVariant data(int role) const;
    QString message;
};

class MessageLogModel : public AbstractLoggerModel
{
    Q_OBJECT
    Q_PROPERTY(QString phoneNumber
               READ phoneNumber
               WRITE setPhoneNumber
               NOTIFY phoneNumberChanged)
public:
    enum MessageLogRoles {
        Message = AbstractLoggerModel::LastLogRole,
        Date
    };

    explicit MessageLogModel(QContactManager *manager, QObject *parent = 0);
    QString phoneNumber() const;
    void setPhoneNumber(QString value);

    Q_INVOKABLE void appendMessage(const QString &number, const QString &message, bool incoming);

public slots:
    void onMessageReceived(const QString &number, const QString &message);
    void onMessageSent(const QString &number, const QString &message);

signals:
    void phoneNumberChanged();
    
protected:
    LogEntry *createEntry(const Tpl::EventPtr &event);
    void handleEntities(const Tpl::EntityPtrList &entities);

private:
    QString mPhoneNumber;
};

#endif // MESSAGELOGMODEL_H
