.pragma library

var CONTEXT_TYPE_HOME       = 0
var CONTEXT_TYPE_WORK       = 1
var CONTEXT_TYPE_OTHER      = 2

var DETAIL_TYPE_ADDRESS       = 1
var DETAIL_TYPE_ONLINEACCOUNT = 18
var DETAIL_TYPE_PHONENUMBER   = 20

var PHONENUMBER_SUBTYPE_LANDLINE  = 0
var PHONENUMBER_SUBTYPE_MOBILE    = 1
var PHONENUMBER_SUBTYPE_FAX = 2
var PHONENUMBER_SUBTYPE_PAGER = 3
var PHONENUMBER_SUBTYPE_VOICE = 4
var PHONENUMBER_SUBTYPE_MODEM = 5
var PHONENUMBER_SUBTYPE_VIDEO = 6
var PHONENUMBER_SUBTYPE_CAR = 7
var PHONENUMBER_SUBTYPE_BULLETINBOARDSYSTEM = 8
var PHONENUMBER_SUBTYPE_MESSAGINGCAPABLE = 9
var PHONENUMBER_SUBTYPE_ASSISTANT = 10
var PHONENUMBER_SUBTYPE_DTMFMENU = 11

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
    if (detail.type == DETAIL_TYPE_PHONENUMBER) {
        if (detail.contexts.indexOf(CONTEXT_TYPE_HOME) > -1) {
            if (detail.subTypes.indexOf(PHONENUMBER_SUBTYPE_VOICE) > -1 || detail.subTypes.isEmpty) {
                return "Home";
            } else if (detail.subTypes.indexOf(PHONENUMBER_SUBTYPE_FAX) > -1) {
                return "Home Fax";
            }
        } else if (detail.contexts.indexOf(CONTEXT_TYPE_WORK) > -1) {
            if (detail.subTypes.indexOf(PHONENUMBER_SUBTYPE_VOICE) > -1 || detail.subTypes.isEmpty) {
                return "Work";
            } else if (detail.subTypes.indexOf(PHONENUMBER_SUBTYPE_FAX) > -1) {
                return "Work Fax";
            }
        } else if (detail.subTypes.indexOf(PHONENUMBER_SUBTYPE_MOBILE) > -1) {
            return "Mobile";
        } else if (detail.subTypes.indexOf(PHONENUMBER_SUBTYPE_PAGER) > -1) {
            return "Pager";
        }

        return "Other";
    } else if (detail.type == DETAIL_TYPE_ONLINEACCOUNT) {
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
    } else if (detail.type == DETAIL_TYPE_ADDRESS) {
        var contexts = detail.contexts
        if (contexts.indexOf(CONTEXT_TYPE_HOME) > -1) {
            return ADDRESS_LABEL_HOME;
        } else if (contexts.indexOf(CONTEXT_TYPE_WORK) > -1) {
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

        if (context == CONTEXT_TYPE_HOME) {
            return "Home";
        } else if (context == CONTEXT_TYPE_WORK) {
            return "Work";
        } else if (subType == CONTEXT_TYPE_OTHER) {
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
    if (detail.type == DETAIL_TYPE_PHONENUMBER) {
        if (newSubType == "Home") {
            detail.contexts = [ CONTEXT_TYPE_HOME ];
            detail.subTypes = [ PHONENUMBER_SUBTYPE_VOICE ];
        } else if (newSubType == "Work") {
            detail.contexts = [ CONTEXT__TYPE_WORK ];
            detail.subTypes = [ PHONENUMBER_SUBTYPE_VOICE ];
        } else if (newSubType == "Work Fax") {
            detail.contexts = [ CONTEXT_TYPE_WORK ];
            detail.subTypes = [ PHONENUMBER_SUBTYPE_FAX ];
        } else if (newSubType == "Home Fax") {
            detail.contexts = [ CONTEXT_TYPE_HOME ];
            detail.subTypes = [ PHONENUMBER_SUBTYPE_FAX ];
        } else if (newSubType == "Mobile") {
            detail.contexts = [ ];
            detail.subTypes = [ PHONENUMBER_SUBTYPE_MOBILE ];
        } else if (newSubType == "Pager") {
            detail.contexts = [ ];
            detail.subTypes = [ PHONENUMBER_SUBTYPE_PAGER ];
        } else {
            detail.contexts = [ CONTEXT_TYPE_OTHER ];
            detail.subTypes = [ ];
        }
    } else if (detail.type == DETAIL_TYPE_ONLINEACCOUNT) {
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
    } else if (detail.type == DETAIL_TYPE_ADDRESS) {
        if (newSubType == ADDRESS_LABEL_HOME) {
            detail.contexts = [ CONTEXT_TYPE_HOME ];
        } else if (newSubType == ADDRESS_LABEL_WORK) {
            detail.contexts = [ CONTEXT_TYPE_WORK ];
        } else {
            detail.contexts = [ CONTEXT_TYPE_OTHER ];
        }
    } else {
        var context = -1
        if (newSubType == "Home") {
            context = CONTEXT_TYPE_HOME;
        } else if (newSubType == "Work") {
            context = CONTEXT_TYPE_WORK;
        } else {
            context = CONTEXT_TYPE_OTHER;
        }

        updateContext(detail, context);
    }
}

