/*
 * Copyright (C) 2013-2015 Canonical, Ltd.
 *
 * Authors:
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

#ifndef CONTACTWATCHER_H
#define CONTACTWATCHER_H

#include <QObject>
#include <QContactManager>
#include <QContactAbstractRequest>
#include <QContactFetchRequest>
#include <QQmlParserStatus>

QTCONTACTS_USE_NAMESPACE

class ContactWatcher : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QString contactId READ contactId NOTIFY contactIdChanged)
    Q_PROPERTY(QString avatar READ avatar NOTIFY avatarChanged)
    Q_PROPERTY(QString alias READ alias NOTIFY aliasChanged)
    Q_PROPERTY(QString identifier READ identifier WRITE setIdentifier NOTIFY identifierChanged)
    Q_PROPERTY(QString phoneNumber READ identifier WRITE setIdentifier NOTIFY identifierChanged)
    // The details property changes according to the detail type.
    // One property is always present on the map though, the "detailType" property.
    Q_PROPERTY(QVariantMap detailProperties READ detailProperties NOTIFY detailPropertiesChanged)
    Q_PROPERTY(bool isUnknown READ isUnknown NOTIFY isUnknownChanged)
    Q_PROPERTY(bool interactive READ interactive NOTIFY interactiveChanged)
    Q_PROPERTY(QStringList addressableFields READ addressableFields WRITE setAddressableFields NOTIFY addressableFieldsChanged)

public:
    explicit ContactWatcher(QObject *parent = 0);
    ~ContactWatcher();

    QString contactId() const;
    QString avatar() const;
    QString alias() const;
    QString identifier() const;
    void setIdentifier(const QString &identifier);
    QVariantMap detailProperties() const;
    bool isUnknown() const;
    bool interactive() const;

    // defaults to only phone number searching
    QStringList addressableFields() const;
    void setAddressableFields(const QStringList &fields);

    void classBegin();
    void componentComplete();

    // helpers
    Q_INVOKABLE QVariantList wrapIntList(const QList<int> &list);
    Q_INVOKABLE QList<int> unwrapIntList(const QVariantList &list);

Q_SIGNALS:
    void contactIdChanged();
    void avatarChanged();
    void aliasChanged();
    void identifierChanged();
    void detailPropertiesChanged();
    void isUnknownChanged();
    void interactiveChanged();
    void addressableFieldsChanged();

protected Q_SLOTS:
    void onContactsAdded(QList<QContactId> ids);
    void onContactsChanged(QList<QContactId> ids);
    void onContactsRemoved(QList<QContactId> ids);
    void onResultsAvailable();
    void onRequestStateChanged(QContactAbstractRequest::State state);

private:
    void startSearching();
    bool checkPhoneNumberMatch(const QString &phoneNumber);

    QContactFetchRequest *mRequest;
    QContactId mContactId;
    QString mAvatar;
    QString mAlias;
    QString mIdentifier;
    QVariantMap mDetailProperties;
    bool mInteractive;
    bool mCompleted;
    QStringList mAddressableFields;
};

#endif // CONTACTWATCHER_H
