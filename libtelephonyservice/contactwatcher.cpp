/*
 * Copyright (C) 2013-2017 Canonical, Ltd.
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

#include "contactwatcher.h"
#include "contactutils.h"
#include "phoneutils.h"
#include "accountentry.h"
#include "telepathyhelper.h"
#include <QContactManager>
#include <QContactFetchByIdRequest>
#include <QContactFetchRequest>
#include <QContactAvatar>
#include <QContactExtendedDetail>
#include <QContactPhoneNumber>
#include <QContactDetailFilter>
#include <QContactIntersectionFilter>
#include <QContactUnionFilter>
#include <QContactOnlineAccount>

namespace C {
#include <libintl.h>
}

ContactWatcher::ContactWatcher(QObject *parent) :
    QObject(parent), mRequest(0), mInteractive(false), mCompleted(false)
{
    QContactManager *manager = ContactUtils::sharedManager();
    QObject::connect(manager, &QContactManager::contactsAdded,
                     this, &ContactWatcher::onContactsAdded);
    QObject::connect(manager, &QContactManager::contactsChanged,
                     this, &ContactWatcher::onContactsChanged);
    QObject::connect(manager, &QContactManager::contactsRemoved,
                     this, &ContactWatcher::onContactsRemoved);

    connect(this, SIGNAL(contactIdChanged()), SIGNAL(isUnknownChanged()));
}

ContactWatcher::~ContactWatcher()
{
    if (mRequest) {
        mRequest->cancel();
        mRequest->deleteLater();
    }
}

QContactIntersectionFilter ContactWatcher::filterForField(const QString &field, const QString &identifier)
{
    QContactIntersectionFilter intersectionFilter;

    if (field == "X-IRC") {
        QContactDetailFilter nameFilter = QContactDetailFilter();
        nameFilter.setDetailType(QContactOnlineAccount::Type, QContactOnlineAccount::FieldProtocol);
        nameFilter.setMatchFlags(QContactFilter::MatchExactly);
        nameFilter.setValue(QContactOnlineAccount::ProtocolIrc);

        QContactDetailFilter valueFilter = QContactDetailFilter();
        valueFilter.setDetailType(QContactOnlineAccount::Type, QContactOnlineAccount::FieldAccountUri);
        valueFilter.setMatchFlags(QContactFilter::MatchExactly);
        valueFilter.setValue(identifier);

        intersectionFilter.append(nameFilter);
        intersectionFilter.append(valueFilter);
    }

    return intersectionFilter;
}

void ContactWatcher::startSearching()
{
    if (!mCompleted || mIdentifier.isEmpty() || !mInteractive || mAddressableFields.isEmpty()) {
        // component is not ready yet or no identifier given,
        // or the number is not interactive and thus doesn't need contact info at all
        return;
    }

    // cancel current request if necessary
    if (mRequest) {
        mRequest->cancel();
        mRequest->deleteLater();
    }

    // FIXME: search for all the fields
    mRequest = new QContactFetchRequest(this);

    QContactUnionFilter topLevelFilter;
    Q_FOREACH(const QString &field, mAddressableFields) {
        if (field == "tel") {
            topLevelFilter.append(QContactPhoneNumber::match(mIdentifier));
        } else {
            QContactIntersectionFilter intersectionFilter;
            // try a special filter
            intersectionFilter = filterForField(field, mIdentifier);
            if (intersectionFilter.filters().isEmpty()) {
                // FIXME: handle more fields
                // rely on a generic field filter
                QContactDetailFilter nameFilter = QContactDetailFilter();
                nameFilter.setDetailType(QContactExtendedDetail::Type, QContactExtendedDetail::FieldName);
                nameFilter.setMatchFlags(QContactFilter::MatchExactly);
                nameFilter.setValue(field);

                QContactDetailFilter valueFilter = QContactDetailFilter();
                valueFilter.setDetailType(QContactExtendedDetail::Type, QContactExtendedDetail::FieldData);
                valueFilter.setMatchFlags(QContactFilter::MatchExactly);
                valueFilter.setValue(mIdentifier);

                intersectionFilter.append(nameFilter);
                intersectionFilter.append(valueFilter);
            }

            topLevelFilter.append(intersectionFilter);
        }
    }

    mRequest->setFilter(topLevelFilter);
    connect(mRequest, SIGNAL(stateChanged(QContactAbstractRequest::State)),
                      SLOT(onRequestStateChanged(QContactAbstractRequest::State)));
    connect(mRequest, SIGNAL(resultsAvailable()), SLOT(onResultsAvailable()));
    mRequest->setManager(ContactUtils::sharedManager());
    mRequest->start();
}

void ContactWatcher::clear()
{
    setAlias(QString::null);
    setContactId(QString::null);
    setAvatar(QString::null);
    setDetailProperties(QVariantMap());
}

void ContactWatcher::updateAlias()
{
    if (mIdentifier.isEmpty()) {
        setAlias(QString::null);
    } else if (mIdentifier.startsWith(OFONO_PRIVATE_NUMBER)) {
        setAlias(C::dgettext("telephony-service", "Private Number"));
    } else if (mIdentifier.startsWith(OFONO_UNKNOWN_NUMBER)) {
        setAlias(C::dgettext("telephony-service", "Unknown Number"));
    }
}

QVariantList ContactWatcher::wrapIntList(const QList<int> &list)
{
    QVariantList resultList;
    Q_FOREACH(int value, list) {
        resultList << value;
    }
    return resultList;
}

QList<int> ContactWatcher::unwrapIntList(const QVariantList &list)
{
    QList<int> resultList;
    Q_FOREACH(const QVariant &value, list) {
        resultList << value.toInt();
    }
    return resultList;
}

QString ContactWatcher::contactId() const
{
    return mContactId;
}

void ContactWatcher::setContactId(const QString &id)
{
    if (id == mContactId) {
        return;
    }

    if (id == QStringLiteral("qtcontacts:::")) {
        mContactId = QString::null;
    } else {
        mContactId = id;
    }
    Q_EMIT contactIdChanged();
}

QString ContactWatcher::avatar() const
{
    return mAvatar;
}

void ContactWatcher::setAvatar(const QString &avatar)
{
    if (avatar == mAvatar) {
        return;
    }
    mAvatar = avatar;
    Q_EMIT avatarChanged();
}

QString ContactWatcher::alias() const
{
    return mAlias;
}

void ContactWatcher::setAlias(const QString &alias)
{
    if (alias == mAlias) {
        return;
    }
    mAlias = alias;
    Q_EMIT aliasChanged();
}

QString ContactWatcher::identifier() const
{
    return mIdentifier;
}

void ContactWatcher::setIdentifier(const QString &identifier)
{
    if (mIdentifier == identifier) {
        return;
    }

    // FIXME: ofono stuff, maybe move somewhere else?
    const bool isPrivate = identifier.startsWith(OFONO_PRIVATE_NUMBER);
    const bool isUnknown = identifier.startsWith(OFONO_UNKNOWN_NUMBER);
    const bool isInteractive = !identifier.isEmpty() && !isPrivate && !isUnknown;

    mIdentifier = identifier;
    if (isInteractive != mInteractive) {
        mInteractive = isInteractive;
        Q_EMIT interactiveChanged();
    }

    mIdentifier = normalizeIdentifier(mIdentifier);
    Q_EMIT identifierChanged();


    if (mIdentifier.isEmpty() || isPrivate || isUnknown) {
        updateAlias();
        setContactId(QString::null);
        setAvatar(QString::null);
        setDetailProperties(QVariantMap());
    } else {
        startSearching();
    }
}

QVariantMap ContactWatcher::detailProperties() const
{
    return mDetailProperties;
}

void ContactWatcher::setDetailProperties(const QVariantMap &properties)
{
    if (properties == mDetailProperties) {
        return;
    }
    mDetailProperties = properties;
    Q_EMIT detailPropertiesChanged();
}

bool ContactWatcher::isUnknown() const
{
    return mContactId.isNull();
}

bool ContactWatcher::interactive() const
{
    return mInteractive;
}

QStringList ContactWatcher::addressableFields() const
{
    return mAddressableFields;
}

void ContactWatcher::setAddressableFields(const QStringList &fields)
{
    mAddressableFields = fields;
    Q_EMIT addressableFieldsChanged();

    startSearching();
}

QString ContactWatcher::normalizeIdentifier(const QString &identifier, bool incoming)
{
    QString finalId = identifier;
    // FIXME: this is a hack, we need to find a better way of matching contacts for accounts
    // that don't have addressable fields
    if (finalId.startsWith("sip:")) {
        finalId.remove("sip:").remove(QRegularExpression("@.*$"));

        // If the final ID's length is bigger than 6 digits, we assume it is a phone number.
        // For incoming phone numbers, assume they come in the full format, including country code
        // and just append the + to the beginning.
        // FIXME: this rule might be an over-simplification of the cases. Change it to a more complete
        // approach if the need appears.
        if (!finalId.startsWith("+") && finalId.length() > 6 && incoming) {
            finalId.prepend("+");
        }
    }
    return finalId;
}

void ContactWatcher::classBegin()
{
}

void ContactWatcher::componentComplete()
{
    mCompleted = true;
    updateAlias();
    startSearching();
}

void ContactWatcher::onContactsAdded(const QList<QContactId> &ids)
{
    // ignore this signal if we have a contact already
    // or if we have no phone number set
    if (!mContactId.isNull() || mIdentifier.isEmpty()) {
        return;
    }

    startSearching();
}

void ContactWatcher::onContactsChanged(const QList<QContactId> &ids)
{
    // check for changes even if we have this contact already,
    // as the number might have changed, thus invalidating the current contact
    startSearching();
}

void ContactWatcher::onContactsRemoved(const QList<QContactId> &ids)
{
    Q_FOREACH(const QContactId &id, ids) {
        if (id.toString() == mContactId) {
            clear();

            startSearching();
            break;
        }
    }
}

void ContactWatcher::onResultsAvailable()
{
    QContactFetchRequest *request = qobject_cast<QContactFetchRequest*>(sender());
    if (request && request->contacts().size() > 0) {
        QContact contact;
        // iterate over all contacts
        Q_FOREACH(const QString &field, mAddressableFields) {
            if (!contact.isEmpty()) {
                break;
            }
            if (field == "tel") {
                Q_FOREACH(const QContact &resultContact, request->contacts()) {
                    Q_FOREACH(const QContactPhoneNumber phoneNumber, resultContact.details(QContactDetail::TypePhoneNumber)) {
                        if (PhoneUtils::comparePhoneNumbers(phoneNumber.number(), mIdentifier) > PhoneUtils::NO_MATCH) {
                            contact = resultContact;
                            break;
                        }
                    }
                    if (!contact.isEmpty()) {
                        break;
                    }
                }
                if (!contact.isEmpty()) {
                    break;
                }
            } else {
                // FIXME: add proper support for non-phonenumber ids
                contact = request->contacts().at(0);
                break;
            }
        }

        setContactId(contact.id().toString());
        setAvatar(contact.detail<QContactAvatar>().imageUrl().toString());
        setAlias(ContactUtils::formatContactName(contact));

        QVariantMap detailProperties;
        Q_FOREACH(const QString &field, mAddressableFields) {
            if (field == "tel") {
                Q_FOREACH(const QContactPhoneNumber phoneNumber, contact.details(QContactDetail::TypePhoneNumber)) {
                    if (PhoneUtils::comparePhoneNumbers(phoneNumber.number(), mIdentifier) > PhoneUtils::NO_MATCH) {
                        detailProperties["type"] = (int)QContactDetail::TypePhoneNumber;
                        detailProperties["phoneNumberSubTypes"] = wrapIntList(phoneNumber.subTypes());
                        detailProperties["phoneNumberContexts"] = wrapIntList(phoneNumber.contexts());
                        break;
                    }
                }
            } else {
                // FIXME: add proper support for more fields
            }
        }
        setDetailProperties(detailProperties);
    }
}

void ContactWatcher::onRequestStateChanged(QContactAbstractRequest::State state)
{
    QContactFetchRequest *request = mRequest;
    if (request && state == QContactAbstractRequest::FinishedState) {
        mRequest = 0;
        request->deleteLater();

        // if we got no results and we had a contact previously, we need to clear the data
        if (request->contacts().isEmpty() && !mContactId.isNull()) {
            clear();
        }
    }
}
