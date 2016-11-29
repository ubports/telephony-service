/*
 * Copyright (C) 2013-2016 Canonical, Ltd.
 *
 * Authors:
 *  Michael Terry <michael.terry@canonical.com>
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#include "greetercontacts.h"

#include <pwd.h>
#include <QContactAvatar>
#include <QContactInvalidFilter>
#include <QContactManagerEngine>
#include <QContactName>
#include <QContactDisplayLabel>
#include <QContactPhoneNumber>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMutexLocker>
#include <unistd.h>

QTCONTACTS_USE_NAMESPACE

GreeterContacts *GreeterContacts::instance()
{
    static GreeterContacts *self = new GreeterContacts();
    return self;
}

GreeterContacts::GreeterContacts(QObject *parent)
: QObject(parent),
  mActiveUser(),
  mFilter(QContactInvalidFilter()),
  mContacts()
{
    qDBusRegisterMetaType<QMap<QString, QString> >();
    // Watch for changes
    QDBusConnection connection = QDBusConnection::AS_BUSNAME();
    connection.connect("org.freedesktop.Accounts",
                       nullptr,
                       "org.freedesktop.DBus.Properties",
                       "PropertiesChanged",
                       this,
                       SLOT(accountsPropertiesChanged(QString, QVariantMap, QStringList, QDBusMessage)));

    // Are we in greeter mode or not?
    if (isGreeterMode()) {
        connection = QDBusConnection::sessionBus();
        connection.connect("com.canonical.UnityGreeter",
                           "/list",
                           "org.freedesktop.DBus.Properties",
                           "PropertiesChanged",
                           this,
                           SLOT(greeterListPropertiesChanged(QString, QVariantMap, QStringList)));

        QDBusInterface iface("org.freedesktop.Accounts",
                             "/org/freedesktop/Accounts",
                             "org.freedesktop.Accounts",
                             QDBusConnection::AS_BUSNAME());
        QDBusPendingCall call = iface.asyncCall("ListCachedUsers");
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher *)),
                this, SLOT(accountsGetUsersReply(QDBusPendingCallWatcher *)));

        queryEntry();
    } else {
        QString uid = QString::number(getuid());
        mActiveUser = "/org/freedesktop/Accounts/User" + uid;
    }

    // get the current value of greeter's isActive property
    connection = QDBusConnection::sessionBus();
    QDBusInterface greeterPropsIface("com.canonical.UnityGreeter",
                                     "/",
                                     "org.freedesktop.DBus.Properties");
    QDBusReply<QVariant> reply = greeterPropsIface.call("Get", "com.canonical.UnityGreeter", "IsActive");
    mGreeterActive = reply.isValid() && reply.value().toBool();
    connection.connect("com.canonical.UnityGreeter",
                       "/",
                       "org.freedesktop.DBus.Properties",
                       "PropertiesChanged",
                       this,
                       SLOT(greeterPropertiesChanged(QString, QVariantMap, QStringList)));
}

GreeterContacts::~GreeterContacts()
{
}

bool GreeterContacts::greeterActive() const
{
    return mGreeterActive;
}

bool GreeterContacts::isGreeterMode()
{
    return qgetenv("XDG_SESSION_CLASS") == "greeter";
}

void GreeterContacts::setContactFilter(const QContactFilter &filter)
{
    QMutexLocker locker(&mMutex);
    mFilter = filter;
    signalIfNeeded();
}

bool GreeterContacts::silentMode()
{
    QMutexLocker locker(&mMutex);
    if (!mSilentMode.isValid()) {
        mSilentMode = getUserValue("com.ubuntu.touch.AccountsService.Sound", "SilentMode");
    }
    return mSilentMode.toBool();
}

QString GreeterContacts::incomingCallSound()
{
    QMutexLocker locker(&mMutex);
    if (!mIncomingCallSound.isValid()) {
        mIncomingCallSound = getUserValue("com.ubuntu.touch.AccountsService.Sound", "IncomingCallSound");
    }
    return mIncomingCallSound.toString();
}

QString GreeterContacts::incomingMessageSound()
{
    QMutexLocker locker(&mMutex);
    if (!mIncomingMessageSound.isValid()) {
        mIncomingMessageSound = getUserValue("com.ubuntu.touch.AccountsService.Sound", "IncomingMessageSound");
    }
    return mIncomingMessageSound.toString();
}

bool GreeterContacts::incomingCallVibrate()
{
    if (silentMode()) {
        QMutexLocker locker(&mMutex);
        return getUserValue("com.ubuntu.touch.AccountsService.Sound", "IncomingCallVibrateSilentMode").toBool();
    }
    QMutexLocker locker(&mMutex);
    if (!mIncomingCallVibrate.isValid()) {
        mIncomingCallVibrate = getUserValue("com.ubuntu.touch.AccountsService.Sound", "IncomingCallVibrate");
    }
    return mIncomingCallVibrate.toBool();
}

bool GreeterContacts::incomingMessageVibrate()
{
    if (silentMode()) {
        QMutexLocker locker(&mMutex);
        return getUserValue("com.ubuntu.touch.AccountsService.Sound", "IncomingMessageVibrateSilentMode").toBool();
    }
    QMutexLocker locker(&mMutex);
    if (!mIncomingMessageVibrate.isValid()) {
        mIncomingMessageVibrate = getUserValue("com.ubuntu.touch.AccountsService.Sound", "IncomingMessageVibrate");
    }
    return mIncomingMessageVibrate.toBool();
}

bool GreeterContacts::dialpadSoundsEnabled()
{
    QMutexLocker locker(&mMutex);
    if (!mDialpadSoundsEnabled.isValid()) {
        mDialpadSoundsEnabled = getUserValue("com.ubuntu.touch.AccountsService.Sound", "DialpadSoundsEnabled");
    }
    return mDialpadSoundsEnabled.toBool();
}

bool GreeterContacts::mmsEnabled()
{
    QMutexLocker locker(&mMutex);
    if (!mMmsEnabled.isValid()) {
        mMmsEnabled = getUserValue("com.ubuntu.touch.AccountsService.Phone", "MmsEnabled");
    }
    return mMmsEnabled.toBool();
}

QString GreeterContacts::defaultSimForCalls()
{
    QMutexLocker locker(&mMutex);
    if (!mDefaultSimForCalls.isValid()) {
        mDefaultSimForCalls = getUserValue("com.ubuntu.touch.AccountsService.Phone", "DefaultSimForCalls");
    }
    return mDefaultSimForCalls.toString();
}

QString GreeterContacts::defaultSimForMessages()
{
    QMutexLocker locker(&mMutex);
    if (!mDefaultSimForMessages.isValid()) {
        mDefaultSimForMessages = getUserValue("com.ubuntu.touch.AccountsService.Phone", "DefaultSimForMessages");
    }
    return mDefaultSimForMessages.toString();
}

QVariantMap GreeterContacts::simNames()
{
    QMutexLocker locker(&mMutex);
    QVariantMap namesAsVariantMap;
    if (!mSimNames.isValid()) {
        QVariant value = getUserValue("com.ubuntu.touch.AccountsService.Phone", "SimNames");
        // the signature is a{ss} instead of a{sv}
        QMap<QString, QString> names = qdbus_cast<QMap<QString, QString> >(value);
        QMapIterator<QString, QString> i(names);
        while (i.hasNext()) {
            i.next();
            namesAsVariantMap[i.key()] = i.value();
        }
        mSimNames = namesAsVariantMap;
    }
    return mSimNames.toMap();
}

void GreeterContacts::greeterListPropertiesChanged(const QString &interface,
                                               const QVariantMap &changed,
                                               const QStringList &invalidated)
{
    if (interface == "com.canonical.UnityGreeter.List") {
        if (changed.contains("ActiveEntry")) {
            updateActiveUser(changed.value("ActiveEntry").toString());
        } else if (invalidated.contains("ActiveEntry")) {
            queryEntry();
        }
    }
}

void GreeterContacts::greeterPropertiesChanged(const QString &interface, const QVariantMap &changed, const QStringList &invalidated)
{
    if (interface == "com.canonical.UnityGreeter") {
        if (changed.contains("IsActive")) {
            mGreeterActive = changed.value("IsActive").toBool();
            Q_EMIT greeterActiveChanged();
        }
    }
}

void GreeterContacts::setMmsEnabled(bool enabled)
{
    QString uid = QString::number(getuid());
    QDBusInterface iface("org.freedesktop.Accounts",
                         "/org/freedesktop/Accounts/User" + uid,
                         "org.freedesktop.DBus.Properties",
                         QDBusConnection::AS_BUSNAME());
    iface.asyncCall("Set", "com.ubuntu.touch.AccountsService.Phone", "MmsEnabled", QVariant::fromValue(QDBusVariant(enabled)));
}

void GreeterContacts::setDefaultSimForMessages(const QString &objPath)
{
    QString uid = QString::number(getuid());
    QDBusInterface iface("org.freedesktop.Accounts",
                         "/org/freedesktop/Accounts/User" + uid,
                         "org.freedesktop.DBus.Properties",
                         QDBusConnection::AS_BUSNAME());
    iface.asyncCall("Set", "com.ubuntu.touch.AccountsService.Phone", "DefaultSimForMessages", QVariant::fromValue(QDBusVariant(objPath)));
}

void GreeterContacts::setDefaultSimForCalls(const QString &objPath)
{
    QString uid = QString::number(getuid());
    QDBusInterface iface("org.freedesktop.Accounts",
                         "/org/freedesktop/Accounts/User" + uid,
                         "org.freedesktop.DBus.Properties",
                         QDBusConnection::AS_BUSNAME());
    iface.asyncCall("Set", "com.ubuntu.touch.AccountsService.Phone", "DefaultSimForCalls", QVariant::fromValue(QDBusVariant(objPath)));
}

void GreeterContacts::setSimNames(const QVariantMap &simNames)
{
    QMap<QString, QString> newSimNames;

    QMapIterator<QString, QVariant> i(simNames);
    while (i.hasNext()) {
        i.next();
        newSimNames[i.key()] = i.value().toString();
    }

    QString uid = QString::number(getuid());
    QDBusInterface iface("org.freedesktop.Accounts",
                         "/org/freedesktop/Accounts/User" + uid,
                         "org.freedesktop.DBus.Properties",
                         QDBusConnection::AS_BUSNAME());
    iface.asyncCall("Set", "com.ubuntu.touch.AccountsService.Phone", "SimNames", QVariant::fromValue(QDBusVariant(QVariant::fromValue(newSimNames))));
}

QVariant GreeterContacts::getUserValue(const QString &interface, const QString &propName)
{
    QDBusInterface iface("org.freedesktop.Accounts",
                         mActiveUser,
                         "org.freedesktop.DBus.Properties",
                         QDBusConnection::AS_BUSNAME());
    QDBusReply<QVariant> reply = iface.call("Get", interface, propName);
    if (reply.isValid()) {
        return reply.value();
    } else {
        qWarning() << "Failed to get user property " << propName << " from AccountsService:" << reply.error().message();
    }
    return QVariant();
}

void GreeterContacts::checkUpdatedValue(const QVariantMap &changed,
                                        const QStringList &invalidated,
                                        const QString &propName,
                                        QVariant &propValue)
{
    if (changed.contains(propName)) {
        propValue = changed.value(propName);
    } else if (invalidated.contains(propName)) {
        propValue = QVariant();
    }
}

void GreeterContacts::accountsPropertiesChanged(const QString &interface,
                                                const QVariantMap &changed,
                                                const QStringList &invalidated,
                                                const QDBusMessage &message)
{
    if (interface == "com.canonical.TelephonyServiceApprover") {
        if (changed.contains("CurrentContact")) {
            mContacts.insert(message.path(), qdbus_cast<QVariantMap>(changed.value("CurrentContact")));
            signalIfNeeded();
        } else if (invalidated.contains("CurrentContact")) {
            queryContact(message.path());
        }
    } else if (interface == "com.ubuntu.touch.AccountsService.Sound" &&
               message.path() == mActiveUser) {
        checkUpdatedValue(changed, invalidated, "SilentMode", mSilentMode);
        checkUpdatedValue(changed, invalidated, "IncomingCallSound", mIncomingCallSound);
        checkUpdatedValue(changed, invalidated, "IncomingMessageSound", mIncomingMessageSound);
        checkUpdatedValue(changed, invalidated, "IncomingMessageVibrate", mIncomingMessageVibrate);
        checkUpdatedValue(changed, invalidated, "IncomingCallVibrate", mIncomingCallVibrate);
        checkUpdatedValue(changed, invalidated, "DialpadSoundsEnabled", mDialpadSoundsEnabled);
    } else if (interface == "com.ubuntu.touch.AccountsService.Phone" &&
               message.path() == mActiveUser) {
        checkUpdatedValue(changed, invalidated, "DefaultSimForCalls", mDefaultSimForCalls);
        checkUpdatedValue(changed, invalidated, "DefaultSimForMessages", mDefaultSimForMessages);
        checkUpdatedValue(changed, invalidated, "MmsEnabled", mMmsEnabled);
        checkUpdatedValue(changed, invalidated, "SimNames", mSimNames);
        Q_FOREACH(const QString &key, changed.keys()) {
            Q_EMIT phoneSettingsChanged(key);
        }
        Q_FOREACH(const QString &key, invalidated) {
            Q_EMIT phoneSettingsChanged(key);
        }
    }
}

void GreeterContacts::greeterGetEntryReply(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<QVariant> reply = *watcher;
    if (!reply.isError()) {
        updateActiveUser(reply.argumentAt<0>().toString());
    } else {
        qWarning() << "Failed to get active entry from Unity Greeter:" << reply.error().message();
    }
    watcher->deleteLater();
}

void GreeterContacts::accountsGetUsersReply(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<QList<QDBusObjectPath>> reply = *watcher;
    if (!reply.isError()) {
        Q_FOREACH (const QDBusObjectPath &user, reply.argumentAt<0>()) {
            queryContact(user.path());
        }
    } else {
        qWarning() << "Failed to get user list from AccountsService:" << reply.error().message();
    }
    watcher->deleteLater();
}

void GreeterContacts::accountsGetContactReply(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<QVariant> reply = *watcher;
    if (!reply.isError()) {
        mContacts.insert(watcher->property("telepathyPath").toString(), qdbus_cast<QVariantMap>(reply.argumentAt<0>()));
        signalIfNeeded();
    } else {
        qWarning() << "Failed to get user's contact from AccountsService:" << reply.error().message();
    }
    watcher->deleteLater();
}

void GreeterContacts::queryEntry()
{
    QDBusInterface iface("com.canonical.UnityGreeter",
                         "/list",
                         "org.freedesktop.DBus.Properties",
                         QDBusConnection::sessionBus());
    QDBusPendingCall call = iface.asyncCall("Get", "com.canonical.UnityGreeter.List", "ActiveEntry");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher *)),
            this, SLOT(greeterGetEntryReply(QDBusPendingCallWatcher *)));
}

void GreeterContacts::queryContact(const QString &user)
{
    QDBusInterface iface("org.freedesktop.Accounts",
                         user,
                         "org.freedesktop.DBus.Properties",
                         QDBusConnection::AS_BUSNAME());
    QDBusPendingCall call = iface.asyncCall("Get", "com.canonical.TelephonyServiceApprover", "CurrentContact");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    watcher->setProperty("telepathyPath", QVariant(user));
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher *)),
            this, SLOT(accountsGetContactReply(QDBusPendingCallWatcher *)));
}

void GreeterContacts::updateActiveUser(const QString &username)
{
    struct passwd *pwinfo = getpwnam(username.toLatin1());
    if (pwinfo) {
        mActiveUser = "/org/freedesktop/Accounts/User" + QString::number(pwinfo->pw_uid);
        mSilentMode = QVariant();
        mIncomingCallSound = QVariant();
        mIncomingMessageSound = QVariant();
        mIncomingCallVibrate = QVariant();
        mIncomingMessageVibrate = QVariant();
        mDialpadSoundsEnabled = QVariant();
        mMmsEnabled = QVariant();
        mDefaultSimForCalls = QVariant();
        mDefaultSimForMessages = QVariant();
        mSimNames = QVariant();
        signalIfNeeded();
    }
}

QContact GreeterContacts::lookupContact()
{
    // For now, only ever look at active user's contact info.  In future,
    // maybe we should search all users for any matching info.
    QVariantMap contactInfo = mContacts.value(mActiveUser);
    if (!contactInfo.empty()) {
        QContact contact = mapToContact(contactInfo);
        if (QContactManagerEngine::testFilter(mFilter, contact)) {
            return contact;
        }
    }

    return QContact();
}

void GreeterContacts::signalIfNeeded()
{
    QContact contact = lookupContact();
    if (!contact.isEmpty()) {
        Q_EMIT contactUpdated(contact);
    }
}

void GreeterContacts::emitContact(const QContact &contact)
{
    QString uid = QString::number(getuid());
    QVariantMap map = contactToMap(contact);

    if (!map.value("Image").toString().isEmpty()) {
        // OK, so we want to tell LightDM about our contact.  But LightDM won't
        // have access to our image file in their normal location managed by
        // evolution.  And rather than give world-readable permissions to our
        // evolution dir, we minimize the damage by copying the image to a new
        // more accessible location.

        // Clean up from previous (poor) implementation of this method
        QFile imageFile(QDir::home().filePath(".telephony-service-contact-image"));
        imageFile.remove();

        // Now copy into greeter data dir, if one is set
        QString path = qgetenv("XDG_GREETER_DATA_DIR");
        if (!path.isEmpty()) {
            QDir(path).mkdir("telephony-service"); // create namespaced subdir
            path += "/telephony-service/contact-image";
            QFile(path).remove(); // copy() won't overwrite, so remove before
            if (QFile(map.value("Image").toString()).copy(path)) {
                map.insert("Image", path);
            }
        }
    }

    QDBusInterface iface("org.freedesktop.Accounts",
                         "/org/freedesktop/Accounts/User" + uid,
                         "org.freedesktop.DBus.Properties",
                         QDBusConnection::AS_BUSNAME());
    iface.asyncCall("Set", "com.canonical.TelephonyServiceApprover", "CurrentContact", QVariant::fromValue(QDBusVariant(QVariant(map))));
}

QVariantMap GreeterContacts::contactToMap(const QContact &contact)
{
    QVariantMap map;

    QContactAvatar avatarDetail = contact.detail<QContactAvatar>();
    map.insert("Image", avatarDetail.imageUrl().toLocalFile());

    QContactDisplayLabel displayLabel = contact.detail<QContactDisplayLabel>();
    map.insert("DisplayLabel", displayLabel.label());

    QContactName nameDetail = contact.detail<QContactName>();
    map.insert("FirstName", nameDetail.firstName());
    map.insert("MiddleName", nameDetail.middleName());
    map.insert("LastName", nameDetail.lastName());

    QContactPhoneNumber numberDetail = contact.detail<QContactPhoneNumber>();
    map.insert("PhoneNumber", numberDetail.number());

    return map;
}

QContact GreeterContacts::mapToContact(const QVariantMap &map)
{
    QContact contact;

    QContactAvatar avatarDetail;
    avatarDetail.setValue(QContactAvatar::FieldImageUrl, QUrl::fromLocalFile(map.value("Image").toString()));
    contact.saveDetail(&avatarDetail);

    // We only use FirstName and LastName right now in ContactUtils::formatContactName().
    // If/When we use more, we should save more detail values here.
    QContactDisplayLabel displayLabel;
    displayLabel.setValue(QContactDisplayLabel::FieldLabel, map.value("DisplayLabel"));
    contact.saveDetail(&displayLabel);

    QContactName nameDetail;
    nameDetail.setValue(QContactName::FieldFirstName, map.value("FirstName"));
    nameDetail.setValue(QContactName::FieldMiddleName, map.value("MiddleName"));
    nameDetail.setValue(QContactName::FieldLastName, map.value("LastName"));
    contact.saveDetail(&nameDetail);

    QContactPhoneNumber numberDetail;
    numberDetail.setValue(QContactPhoneNumber::FieldNumber, map.value("PhoneNumber"));
    contact.saveDetail(&numberDetail);

    return contact;
}

void GreeterContacts::showGreeter()
{
    QMutexLocker locker(&mMutex);
    QDBusInterface iface("com.canonical.UnityGreeter",
                         "/",
                         "com.canonical.UnityGreeter",
                         QDBusConnection::sessionBus());
    iface.call("ShowGreeter");
}
