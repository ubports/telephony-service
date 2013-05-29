/*
 * Copyright 2012-2013 Canonical Ltd.
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

.import Ubuntu.PhoneApp 0.1 as PhoneApp

var PROTOCOL_LABEL_AIM      = "AIM";
var PROTOCOL_LABEL_MSN      = "Windows Live";
var PROTOCOL_LABEL_YAHOO    = "Yahoo";
var PROTOCOL_LABEL_SKYPE    = "Skype";
var PROTOCOL_LABEL_QQ       = "QQ";
var PROTOCOL_LABEL_GTALK    = "Google Talk";
var PROTOCOL_LABEL_ICQ      = "ICQ";
var PROTOCOL_LABEL_JABBER   = "Jabber";
var PROTOCOL_LABEL_OTHER    = "Other";

var PROTOCOL_TYPE_CUSTOM    = "im";
var PROTOCOL_TYPE_AIM       = "aim";
var PROTOCOL_TYPE_MSN       = "msn";
var PROTOCOL_TYPE_YAHOO     = "yahoo";
var PROTOCOL_TYPE_SKYPE     = "skype";
var PROTOCOL_TYPE_GTALK     = "google_talk";
var PROTOCOL_TYPE_ICQ       = "icq";
var PROTOCOL_TYPE_JABBER    = "jabber";
var PROTOCOL_TYPE_OTHER     = "other";

var detailsSubTypes = [ { value: "Home", label: i18n.tr("Home") },
                        { value: "Work", label: i18n.tr("Work") },
                        { value: "Other", label: i18n.tr("Other") } ];

var phoneSubTypes = [ { value: "Mobile", label: i18n.tr("Mobile") },
                      { value: "Home", label: i18n.tr("Home") },
                      { value: "Work", label: i18n.tr("Work") },
                      { value: "Other", label: i18n.tr("Other") } ];

var emailSubTypes = detailsSubTypes;
var postalAddressSubTypes = detailsSubTypes;


var IMSubTypes = [ { value: PROTOCOL_LABEL_GTALK, label: i18n.tr("Google Talk") },
                   { value: PROTOCOL_LABEL_YAHOO, label: i18n.tr("Yahoo") },
                   { value: PROTOCOL_LABEL_SKYPE, label: i18n.tr("Skype") },
                   { value: PROTOCOL_LABEL_OTHER, label: i18n.tr("Other") } ]

var supportedTypes = [
            {
                name: i18n.tr("Phone"),
                delegateSource: "TextContactDetailsDelegate.qml",
                items: "phoneNumbers",
                newItemText: i18n.tr("Add a phone number"),
                newItemType: "ContactPhoneNumber",
                actionIcon: "../assets/messaging_icon_button.png",
                hasAction: true,
                displayField: "number",
                subTypes: phoneSubTypes,
                showSubtype: true,
                createOnNew: true
            },
            {
                name: i18n.tr("Email"),
                delegateSource: "TextContactDetailsDelegate.qml",
                items: "emails",
                newItemText: i18n.tr("Add an email address"),
                newItemType: "ContactEmailAddress",
                actionIcon: "../assets/contact_icon_email.png",
                hasAction: false,
                displayField: "emailAddress",
                subTypes: emailSubTypes,
                showSubtype: false,
                createOnNew: true
            },
            {
                name: i18n.tr("Address"),
                delegateSource: "AddressContactDetailsDelegate.qml",
                items: "addresses",
                newItemText: i18n.tr("Add a postal address"),
                newItemType: "ContactAddress",
                actionIcon: "../assets/contact_icon_location.png",
                hasAction: false,
                showSubtype: false,
                subTypes: postalAddressSubTypes
            },
            {
                name: i18n.tr("IM"),
                delegateSource: "TextContactDetailsDelegate.qml",
                items: "onlineAccounts",
                newItemText: i18n.tr("Add an online account"),
                displayField: "accountUri",
                newItemType: "ContactOnlineAccount",
                actionIcon: "../assets/contact_icon_IM.png",
                hasAction: false,
                showSubtype: false,
                subTypes: IMSubTypes
            }
        ];

function getTypesWithNoItems(contact) {
    var result = [];
    for (var i = 0; i < supportedTypes.length; i++) {
        var currentType = supportedTypes[i];
        var detailsList = contact[currentType.items];
        /* In newly created contacts the "plural" detail properties (such as "emails")
           do not seem to exist, so allow adding new details for the type in that case. */
        if (detailsList === undefined || detailsList.length == 0) {
            result.push(currentType);
        }
    }
    return result;
}

function getDetailSubType(detail) {
    if (!detail) {
        return "";
    }

    console.debug("Detail:" + detail)

    /* Phone numbers have a special field for the subType */
    if (detail.type == PhoneApp.ContactDetail.PhoneNumber) {
        if (detail.contexts.indexOf(PhoneApp.ContactDetail.ContextHome) > -1) {
            return phoneSubTypes[1];
        } else if (detail.contexts.indexOf(PhoneApp.ContactDetail.ContextWork) > -1) {
            return phoneSubTypes[2];
        } else if (detail.subTypes.indexOf(PhoneApp.ContactPhoneNumber.Mobile) > -1) {
            return phoneSubTypes[0];
        }
        return phoneSubTypes[3];
    } else if (detail.type == PhoneApp.ContactDetail.InstantMessaging) {
        var protocol = detail.protocol;
        if (protocol == PROTOCOL_TYPE_YAHOO) {
            return PROTOCOL_LABEL_YAHOO;
        } else if (protocol == PROTOCOL_TYPE_SKYPE) {
            return PROTOCOL_LABEL_SKYPE;
        } else if (protocol == PROTOCOL_TYPE_GTALK) {
            return PROTOCOL_LABEL_GTALK;
        } else {
            return PROTOCOL_LABEL_OTHER;
        }
    } else if (detail.type == PhoneApp.ContactDetail.Address) {
        var contexts = detail.contexts
        if (contexts.indexOf(PhoneApp.ContactDetail.ContextHome) > -1) {
            return detailsSubTypes[0];
        } else if (contexts.indexOf(PhoneApp.ContactDetail.ContextWork) > -1) {
            return detailsSubTypes[1];
        } else {
            return detailsSubTypes[2];
        }
    } else {
        // The backend supports multiple types but we can just handle one,
        // so let's pick just the first
        var context = -1;
        for (var i = 0; i < detail.contexts.length; i++) {
            context = detail.contexts[i];
            break;
        }
        var subType = -1;
        // not all details have subTypes
        if (detail.subTypes) {
            for (var i = 0; i < detail.subTypes.length; i++) {
                subType = detail.subTypes[i];
                break;
            }
        }

        if (context == PhoneApp.ContactDetail.ContextHome) {
            return detailsSubTypes[0];
        } else if (context == PhoneApp.ContactDetail.ContextWork) {
            return detailsSubTypes[1];
        } else if (subType == PhoneApp.ContactDetail.ContextOther) {
            return detailsSubTypes[2];
        }
    }

    return detailsSubTypes[2];
}

function updateContext(detail, context) {
    // We need a copy because QML list properties can't
    // be directly modified, they need to be reassigned a modified copy.
    detail.contexts = [context];
}

function setDetailSubType(detail, newSubType) {
    if (!detail) {
        return;
    }

    /* Phone numbers have a special field for the subType */
    if (detail.type == PhoneApp.ContactDetail.PhoneNumber) {
        if (newSubType.value == "Home") {
            detail.contexts = [ PhoneApp.ContactDetail.ContextHome ];
            detail.subTypes = [ PhoneApp.ContactPhoneNumber.Voice ];
        } else if (newSubType.value == "Work") {
            detail.contexts = [ PhoneApp.ContactDetail.ContextWork ];
            detail.subTypes = [ PhoneApp.ContactPhoneNumber.Voice ];
        } else if (newSubType.value == "Mobile") {
            detail.contexts = [ ];
            detail.subTypes = [ PhoneApp.ContactPhoneNumber.Mobile ];
        } else {
            detail.contexts = [ PhoneApp.ContactDetail.ContextOther ];
            detail.subTypes = [ ];
        }
    } else if (detail.type == PhoneApp.ContactDetail.InstantMessaging) {
        var protocol = newSubType.value;
        if (protocol == PROTOCOL_LABEL_YAHOO) {
            detail.protocol = PROTOCOL_TYPE_YAHOO;
        } else if (protocol == PROTOCOL_LABEL_SKYPE) {
            detail.protocol = PROTOCOL_TYPE_SKYPE;
        } else if (protocol == PROTOCOL_LABEL_GTALK) {
            detail.protocol = PROTOCOL_TYPE_GTALK;
        } else {
            console.log("Invalid protocol: " + protocol);
            detail.protocol = PROTOCOL_TYPE_OTHER;
        }
    } else if (detail.type == PhoneApp.ContactDetail.Address) {
        if (newSubType.value == "Home") {
            detail.contexts = [ PhoneApp.ContactDetail.ContextHome ];
        } else if (newSubType.value == "Work") {
            detail.contexts = [ PhoneApp.ContactDetail.ContextWork ];
        } else {
            detail.contexts = [ PhoneApp.ContactDetail.ContextOther ];
        }
    } else {
        var context = -1
        if (newSubType.value == "Home") {
            context = PhoneApp.ContactDetail.ContextHome;
        } else if (newSubType.value == "Work") {
            context = PhoneApp.ContactDetail.ContextWork;
        } else {
            context = PhoneApp.ContactDetail.ContextOther;
        }

        updateContext(detail, context);
    }
}

