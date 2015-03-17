/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This file is part of telephony-service.
 *
 * telephony-service is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-service is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QObject>
#include <QStringList>
#include <unistd.h>


#include <QDebug>
bool listGetCalled = false;
bool userGetCalled = false;
QString userPath = "/org/freedesktop/Accounts/User" + QString::number(getuid());

class AccountsInterface : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Accounts")

public:
    AccountsInterface(QObject *parent = 0);

    Q_SCRIPTABLE QList<QDBusObjectPath> ListCachedUsers() const;
};


class TelepathyInterface : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.canonical.TelephonyServiceApprover")

    Q_PROPERTY(QVariantMap CurrentContact READ CurrentContact WRITE SetCurrentContact)

Q_SIGNALS:
    Q_SCRIPTABLE void InitialQueriesDone(); // Only for testing

public:
    TelepathyInterface(QObject *parent = 0);

    QVariantMap CurrentContact();
    void SetCurrentContact(const QVariantMap &map);

    Q_SCRIPTABLE void SetUseInvalidated(bool useInvalidated); // only for testing
    bool mGetCalled; // only for testing

private:
    QVariantMap mCurrentContact;
    bool mUseInvalidated;
};


class ListInterface : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.canonical.UnityGreeter.List")

    Q_PROPERTY(QString ActiveEntry READ ActiveEntry WRITE SetActiveEntry)

public:
    ListInterface(TelepathyInterface *telepathyInterface, QObject *parent = 0);

    QString ActiveEntry() const;
    void SetActiveEntry(const QString &entry);

    Q_SCRIPTABLE void SetUseInvalidated(bool useInvalidated); // only for testing
    bool mGetCalled; // only for testing

private:
    QString mActiveEntry;
    TelepathyInterface *mTelepathyInterface;
    bool mUseInvalidated;
};

class GreeterInterface : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.canonical.UnityGreeter")

    Q_PROPERTY(bool IsActive READ IsActive WRITE SetIsActive)

public:
    GreeterInterface(QObject *parent = 0);

    bool IsActive() const;
    void SetIsActive(bool active);

private:
    bool mIsActive;
};



AccountsInterface::AccountsInterface(QObject *parent)
: QObject(parent)
{
}

QList<QDBusObjectPath> AccountsInterface::ListCachedUsers() const
{
    return QList<QDBusObjectPath>() << QDBusObjectPath(userPath);
}

TelepathyInterface::TelepathyInterface(QObject *parent)
: QObject(parent),
  mCurrentContact(),
  mUseInvalidated(false)
{
}

QVariantMap TelepathyInterface::CurrentContact()
{
    userGetCalled = true;
    if (userGetCalled && listGetCalled)
        Q_EMIT InitialQueriesDone();
    return mCurrentContact;
}

void TelepathyInterface::SetCurrentContact(const QVariantMap &map)
{
    mCurrentContact = map;

    // Now send out a manual changed signal, since Qt won't do it for us.
    QDBusMessage message;
    message = QDBusMessage::createSignal(userPath,
                                         "org.freedesktop.DBus.Properties",
                                         "PropertiesChanged");
    message << "com.canonical.TelephonyServiceApprover";
    if (mUseInvalidated) {
        QStringList invalidatedProps;
        invalidatedProps << "CurrentContact";
        message << QVariantMap();
        message << invalidatedProps;
    } else {
        QVariantMap changedProps;
        changedProps.insert("CurrentContact", QVariant(map));
        message << changedProps;
        message << QStringList();
    }

    QDBusConnection::sessionBus().send(message);
}

void TelepathyInterface::SetUseInvalidated(bool useInvalidated)
{
    mUseInvalidated = useInvalidated;
}

ListInterface::ListInterface(TelepathyInterface *telepathyInterface, QObject *parent)
: QObject(parent),
  mActiveEntry(),
  mTelepathyInterface(telepathyInterface),
  mUseInvalidated(false)
{
}

QString ListInterface::ActiveEntry() const
{
    listGetCalled = true;
    if (userGetCalled && listGetCalled && mTelepathyInterface)
        Q_EMIT mTelepathyInterface->InitialQueriesDone();
    return mActiveEntry;
}

void ListInterface::SetActiveEntry(const QString &entry)
{
    mActiveEntry = entry;

    // Now send out a manual changed signal, since Qt won't do it for us.
    QDBusMessage message;
    message = QDBusMessage::createSignal("/list",
                                         "org.freedesktop.DBus.Properties",
                                         "PropertiesChanged");
    message << "com.canonical.UnityGreeter.List";
    if (mUseInvalidated) {
        QStringList invalidatedProps;
        invalidatedProps << "ActiveEntry";
        message << QVariantMap();
        message << invalidatedProps;
    } else {
        QVariantMap changedProps;
        changedProps.insert("ActiveEntry", QVariant(entry));
        message << changedProps;
        message << QStringList();
    }

    QDBusConnection::sessionBus().send(message);
}

void ListInterface::SetUseInvalidated(bool useInvalidated)
{
    mUseInvalidated = useInvalidated;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QDBusConnection connection = QDBusConnection::sessionBus();

    AccountsInterface accounts;
    connection.registerObject("/org/freedesktop/Accounts", &accounts, QDBusConnection::ExportScriptableContents);

    TelepathyInterface telepathy;
    connection.registerObject(userPath, &telepathy, QDBusConnection::ExportScriptableContents);

    ListInterface list(&telepathy);
    connection.registerObject("/list", &list, QDBusConnection::ExportScriptableContents);

    GreeterInterface greeter;
    connection.registerObject("/", &greeter, QDBusConnection::ExportScriptableContents);

    connection.registerService("com.canonical.UnityGreeter");
    connection.registerService("org.freedesktop.Accounts");

    return a.exec();
}

#include "GreeterContactsTestServer.moc"


GreeterInterface::GreeterInterface(QObject *parent)
: QObject(parent), mIsActive(false)
{

}

bool GreeterInterface::IsActive() const
{
    return mIsActive;
}

void GreeterInterface::SetIsActive(bool active)
{
    mIsActive = active;
    QDBusMessage message;
    message = QDBusMessage::createSignal("/",
                                         "org.freedesktop.DBus.Properties",
                                         "PropertiesChanged");
    message << "com.canonical.UnityGreeter";

    QVariantMap changedProps;
    changedProps.insert("IsActive", QVariant(active));
    message << changedProps;
    message << QStringList();

    QDBusConnection::sessionBus().send(message);
}
