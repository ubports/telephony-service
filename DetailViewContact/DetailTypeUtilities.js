/*
 * Copyright 2012 Canonical Ltd.
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

.pragma library

.import TelephonyApp 0.1 as TelApp

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

var ADDRESS_LABEL_HOME      = "Home";
var ADDRESS_LABEL_WORK      = "Work";
var ADDRESS_LABEL_OTHER     = "Other";

var phoneSubTypes = [ "Mobile", "Home", "Work", "Other" ];

var emailSubTypes = [ "Home", "Work", "Other" ];

var postalAddressSubTypes = [ ADDRESS_LABEL_HOME,
                              ADDRESS_LABEL_WORK,
                              ADDRESS_LABEL_OTHER ];

var IMSubTypes = [ PROTOCOL_LABEL_GTALK,
                   PROTOCOL_LABEL_YAHOO,
                   PROTOCOL_LABEL_SKYPE,
                   PROTOCOL_LABEL_OTHER ];

var supportedTypes = [
            {
                name: "Phone",
                delegateSource: "TextContactDetailsDelegate.qml",
                items: "phoneNumbers",
                newItemText: "Add a phone number",
                newItemType: "ContactPhoneNumber",
                actionIcon: "../assets/messaging_icon_button.png",
                hasAction: true,
                displayField: "number",
                subTypes: phoneSubTypes,
                showSubtype: true,
                createOnNew: true
            },
            {
                name: "Email",
                delegateSource: "TextContactDetailsDelegate.qml",
                items: "emails",
                newItemText: "Add an email address",
                newItemType: "ContactEmailAddress",
                actionIcon: "../assets/contact_icon_email.png",
                hasAction: false,
                displayField: "emailAddress",
                subTypes: emailSubTypes,
                showSubtype: false,
                createOnNew: true
            },
            {
                name: "Address",
                delegateSource: "AddressContactDetailsDelegate.qml",
                items: "addresses",
                newItemText: "Add a postal address",
                newItemType: "ContactAddress",
                actionIcon: "../assets/contact_icon_location.png",
                hasAction: false,
                showSubtype: false,
                subTypes: postalAddressSubTypes
            },
            {
                name: "IM",
                delegateSource: "TextContactDetailsDelegate.qml",
                items: "onlineAccounts",
                newItemText: "Add an online account",
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

    /* Phone numbers have a special field for the subType */
    if (detail.type == TelApp.ContactDetail.PhoneNumber) {
        if (detail.contexts.indexOf(TelApp.ContactDetail.ContextHome) > -1) {
            return "Home";
        } else if (detail.contexts.indexOf(TelApp.ContactDetail.ContextWork) > -1) {
            return "Work";
        } else if (detail.subTypes.indexOf(TelApp.ContactPhoneNumber.Mobile) > -1) {
            return "Mobile";
        }
        return "Other";
    } else if (detail.type == TelApp.ContactDetail.InstantMessaging) {
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
    } else if (detail.type == TelApp.ContactDetail.Address) {
        var contexts = detail.contexts
        if (contexts.indexOf(TelApp.ContactDetail.ContextHome) > -1) {
            return ADDRESS_LABEL_HOME;
        } else if (contexts.indexOf(TelApp.ContactDetail.ContextWork) > -1) {
            return ADDRESS_LABEL_WORK;
        } else {
            return ADDRESS_LABEL_OTHER;
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

        if (context == TelApp.ContactDetail.ContextHome) {
            return "Home";
        } else if (context == TelApp.ContactDetail.ContextWork) {
            return "Work";
        } else if (subType == TelApp.ContactDetail.ContextOther) {
            return "Other";
        }
    }

    return "Other";
}

function updateContext(detail, context) {
    // We need a copy because QML list properties can't
    // be directly modified, they need to be reassigned a modified copy.
    detail.contexts = context;
}

function setDetailSubType(detail, newSubType) {
    if (!detail) {
        return;
    }

    /* Phone numbers have a special field for the subType */
    if (detail.type == TelApp.ContactDetail.PhoneNumber) {
        if (newSubType == "Home") {
            detail.contexts = [ TelApp.ContactDetail.ContextHome ];
            detail.subTypes = [ TelApp.ContactPhoneNumber.Voice ];
        } else if (newSubType == "Work") {
            detail.contexts = [ TelApp.ContactDetail.ContextWork ];
            detail.subTypes = [ TelApp.ContactPhoneNumber.Voice ];
        } else if (newSubType == "Mobile") {
            detail.contexts = [ ];
            detail.subTypes = [ TelApp.ContactPhoneNumber.Mobile ];
        } else {
            detail.contexts = [ TelApp.ContactDetail.ContextOther ];
            detail.subTypes = [ ];
        }
    } else if (detail.type == TelApp.ContactDetail.InstantMessaging) {
        var protocol = newSubType;
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
    } else if (detail.type == TelApp.ContactDetail.Address) {
        if (newSubType == ADDRESS_LABEL_HOME) {
            detail.contexts = [ TelApp.ContactDetail.ContextHome ];
        } else if (newSubType == ADDRESS_LABEL_WORK) {
            detail.contexts = [ TelApp.ContactDetail.ContextWork ];
        } else {
            detail.contexts = [ TelApp.ContactDetail.ContextOther ];
        }
    } else {
        var context = -1
        if (newSubType == "Home") {
            context = TelApp.ContactDetail.ContextHome;
        } else if (newSubType == "Work") {
            context = TelApp.ContactDetail.ContextWork;
        } else {
            context = TelApp.ContactDetail.ContextOther;
        }

        updateContext(detail, context);
    }
}

