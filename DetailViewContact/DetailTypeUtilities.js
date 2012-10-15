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

var phoneSubTypes = [ "Mobile", "Home", "Work", "Work Fax", "Home Fax", "Pager", "Other" ];

var emailSubTypes = [ "Home", "Work", "Other" ];

var postalAddressSubTypes = [ ADDRESS_LABEL_HOME,
                              ADDRESS_LABEL_WORK,
                              ADDRESS_LABEL_OTHER ];

var IMSubTypes = [ PROTOCOL_LABEL_GTALK,
                   PROTOCOL_LABEL_AIM,
                   PROTOCOL_LABEL_MSN,
                   PROTOCOL_LABEL_YAHOO,
                   PROTOCOL_LABEL_SKYPE,
                   PROTOCOL_LABEL_QQ,
                   PROTOCOL_LABEL_ICQ,
                   PROTOCOL_LABEL_JABBER ];

var supportedTypes = [
            {
                name: "Phone",
                delegateSource: "TextContactDetailsDelegate.qml",
                items: "phoneNumbers",
                newItemText: "Add a phone number",
                newItemType: "ContactPhoneNumber",
                actionIcon: "../assets/contact_icon_message.png",
                displayField: "number",
                subTypes: phoneSubTypes,
                createOnNew: true
            },
            {
                name: "Email",
                delegateSource: "TextContactDetailsDelegate.qml",
                items: "emails",
                newItemText: "Add an email address",
                newItemType: "ContactEmailAddress",
                actionIcon: "../assets/contact_icon_email.png",
                displayField: "emailAddress",
                subTypes: emailSubTypes,
                createOnNew: true
            },
            {
                name: "Address",
                delegateSource: "AddressContactDetailsDelegate.qml",
                items: "addresses",
                newItemText: "Add a postal address",
                newItemType: "ContactAddress",
                actionIcon: "../assets/contact_icon_location.png",
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
            if (detail.subTypes.indexOf(TelApp.ContactPhoneNumber.Voice) > -1 || detail.subTypes.isEmpty) {
                return "Home";
            } else if (detail.subTypes.indexOf(TelApp.ContactPhoneNumber.Fax) > -1) {
                return "Home Fax";
            }
        } else if (detail.contexts.indexOf(TelApp.ContactDetail.ContextWork) > -1) {
            if (detail.subTypes.indexOf(TelApp.ContactPhoneNumber.Voice) > -1 || detail.subTypes.isEmpty) {
                return "Work";
            } else if (detail.subTypes.indexOf(TelApp.ContactPhoneNumber.Fax) > -1) {
                return "Work Fax";
            }
        } else if (detail.subTypes.indexOf(TelApp.ContactPhoneNumber.Mobile) > -1) {
            return "Mobile";
        } else if (detail.subTypes.indexOf(TelApp.ContactPhoneNumber.Pager) > -1) {
            return "Pager";
        }

        return "Other";
    } else if (detail.type == TelApp.ContactDetail.InstantMessaging) {
        var protocol = detail.protocol;
        if (protocol == PROTOCOL_TYPE_CUSTOM) {
            if (detail.contexts.indexOf("PROTOCOL=QQ") > -1) {
                return PROTOCOL_LABEL_QQ;
            } else {
                return PROTOCOL_LABEL_OTHER;
            }
        } else if (protocol == PROTOCOL_TYPE_AIM) {
            return PROTOCOL_LABEL_AIM;
        } else if (protocol == PROTOCOL_TYPE_MSN) {
            return PROTOCOL_LABEL_MSN;
        } else if (protocol == PROTOCOL_TYPE_YAHOO) {
            return PROTOCOL_LABEL_YAHOO;
        } else if (protocol == PROTOCOL_TYPE_SKYPE) {
            return PROTOCOL_LABEL_SKYPE;
        } else if (protocol == PROTOCOL_TYPE_GTALK) {
            return PROTOCOL_LABEL_GTALK;
        } else if (protocol == PROTOCOL_TYPE_ICQ) {
            return PROTOCOL_LABEL_ICQ;
        } else if (protocol == PROTOCOL_TYPE_JABBER) {
            return PROTOCOL_LABEL_JABBER;
        } else {
            console.log("Invalid protocol: " + protocol);
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
        for (var i = 0; i < detail.subTypes.length; i++) {
            subType = detail.subTypes[i];
            break;
        }

        if (context == TelApp.ContactDetail.ContextHome) {
            return "Home";
        } else if (context == TelApp.ContactDetail.ContextWork) {
            return "Work";
        } else if (subType == TelApp.ContactDetail.ContextOther) {
            return "Other";
        }
    }

    return "";
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
        } else if (newSubType == "Work Fax") {
            detail.contexts = [ TelApp.ContactDetail.ContextWork ];
            detail.subTypes = [ TelApp.ContactPhoneNumber.Fax ];
        } else if (newSubType == "Home Fax") {
            detail.contexts = [ TelApp.ContactDetail.ContextHome ];
            detail.subTypes = [ TelApp.ContactPhoneNumber.Fax ];
        } else if (newSubType == "Mobile") {
            detail.contexts = [ ];
            detail.subTypes = [ TelApp.ContactPhoneNumber.Mobile ];
        } else if (newSubType == "Pager") {
            detail.contexts = [ ];
            detail.subTypes = [ TelApp.ContactPhoneNumber.Pager ];
        } else {
            detail.contexts = [ TelApp.ContactDetail.ContextOther ];
            detail.subTypes = [ ];
        }
    } else if (detail.type == TelApp.ContactDetail.InstantMessaging) {
        var protocol = newSubType;
        if (protocol == PROTOCOL_LABEL_AIM) {
            detail.protocol = PROTOCOL_TYPE_AIM;
        } else if (protocol == PROTOCOL_LABEL_MSN) {
            detail.protocol = PROTOCOL_TYPE_MSN;
        } else if (protocol == PROTOCOL_LABEL_YAHOO) {
            detail.protocol = PROTOCOL_TYPE_YAHOO;
        } else if (protocol == PROTOCOL_LABEL_SKYPE) {
            detail.protocol = PROTOCOL_TYPE_SKYPE;
        } else if (protocol == PROTOCOL_LABEL_QQ) {
            detail.protocol = PROTOCOL_TYPE_CUSTOM;
            updateContext(detail, "QQ");
        } else if (protocol == PROTOCOL_LABEL_GTALK) {
            detail.protocol = PROTOCOL_TYPE_GTALK;
        } else if (protocol == PROTOCOL_LABEL_ICQ) {
            detail.protocol = PROTOCOL_TYPE_ICQ;
        } else if (protocol == PROTOCOL_LABEL_JABBER) {
            detail.protocol = PROTOCOL_TYPE_JABBER;
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

