.pragma library


var PROTOCOL_LABEL_AIM      = "AIM";
var PROTOCOL_LABEL_MSN      = "Windows Live";
var PROTOCOL_LABEL_YAHOO    = "Yahoo";
var PROTOCOL_LABEL_SKYPE    = "Skype";
var PROTOCOL_LABEL_QQ       = "QQ";
var PROTOCOL_LABEL_GTALK    = "Google Talk";
var PROTOCOL_LABEL_ICQ      = "ICQ";
var PROTOCOL_LABEL_JABBER   = "Jabber";
var PROTOCOL_LABEL_OTHER    = "Other";

var PROTOCOL_TYPE_AIM       = "aim";
var PROTOCOL_TYPE_MSN       = "msn";
var PROTOCOL_TYPE_YAHOO     = "yahoo";
var PROTOCOL_TYPE_SKYPE     = "skype";
var PROTOCOL_TYPE_QQ        = "qq";
var PROTOCOL_TYPE_GTALK     = "google-talk";
var PROTOCOL_TYPE_ICQ       = "icq";
var PROTOCOL_TYPE_JABBER    = "jabber";
var PROTOCOL_TYPE_OTHER     = "other";

var ADDRESS_LABEL_HOME      = "Home";
var ADDRESS_LABEL_WORK      = "Work";
var ADDRESS_LABEL_OTHER     = "Other";

var ADDRESS_TYPE_HOME       = "home";
var ADDRESS_TYPE_WORK       = "work";
var ADDRESS_TYPE_OTHER      = "other";

var phoneSubTypes = [ "Home", "Mobile", "Work", "Work Fax", "Home Fax", "Pager", "Other" ];
var emailSubTypes = [ "Work", "Home", "Mobile", "Other" ];
var postalAddressSubTypes = [ ADDRESS_LABEL_WORK,
                              ADDRESS_LABEL_HOME,
                              ADDRESS_LABEL_OTHER ];
var IMSubTypes = [ PROTOCOL_LABEL_AIM,
                   PROTOCOL_LABEL_MSN,
                   PROTOCOL_LABEL_YAHOO,
                   PROTOCOL_LABEL_SKYPE,
                   PROTOCOL_LABEL_QQ,
                   PROTOCOL_LABEL_GTALK,
                   PROTOCOL_LABEL_ICQ,
                   PROTOCOL_LABEL_JABBER ];
var supportedTypes = [
            {
                name: "Phone",
                delegateSource: "TextContactDetailsDelegate.qml",
                items: "phoneNumbers",
                newItemText: "Add a phone number",
                newItemType: "ContactPhoneNumber",
                actionIcon: "../assets/contact_icon_phone.png",
                displayField: "number",
                subTypes: phoneSubTypes
            },
            {
                name: "Email",
                delegateSource: "TextContactDetailsDelegate.qml",
                items: "emails",
                newItemText: "Add an email address",
                newItemType: "ContactEmailAddress",
                actionIcon: "../assets/contact_icon_email.png",
                displayField: "emailAddress",
                subTypes: emailSubTypes
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
    if (detail.definitionName == "PhoneNumber") {
        if (detail.subTypes.indexOf("home") > -1) {
            if (detail.subTypes.indexOf("voice") > -1) {
                return "Home";
            } else if (detail.subTypes.indexOf("fax") > -1) {
                return "Home Fax";
            }
        } else if (detail.subTypes.indexOf("work") > -1) {
            if (detail.subTypes.indexOf("voice") > -1) {
                return "Work";
            } else if (detail.subTypes.indexOf("fax") > -1) {
                return "Work Fax";
            }
        } else if (detail.subTypes.indexOf("cell") > -1) {
            return "Mobile";
        } else if (detail.subTypes.indexOf("page") > -1) {
            return "Page";
        }

        return "Other";
    } else if (detail.definitionName == "OnlineAccount") {
        var protocol = detail.protocol;
        if (protocol == PROTOCOL_TYPE_AIM) {
            return PROTOCOL_LABEL_AIM;
        } else if (protocol == PROTOCOL_TYPE_MSN) {
            return PROTOCOL_LABEL_MSN;
        } else if (protocol == PROTOCOL_TYPE_YAHOO) {
            return PROTOCOL_LABEL_YAHOO;
        } else if (protocol == PROTOCOL_TYPE_SKYPE) {
            return PROTOCOL_LABEL_SKYPE;
        } else if (protocol == PROTOCOL_TYPE_QQ) {
            return PROTOCOL_LABEL_QQ;
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
    } else if (detail.definitionName == "Address") {
        var subTypes = detail.subTypes
        if (subTypes.indexOf(ADDRESS_TYPE_HOME) > -1) {
            return ADDRESS_LABEL_HOME;
        } else if (subTypes.indexOf(ADDRESS_TYPE_WORK) > -1) {
            return ADDRESS_LABEL_WORK;
        } else {
            return ADDRESS_LABEL_OTHER;
        }
    } else {
        // The backend supports multiple types but we can just handle one,
        // so let's pick just the first
        var type = "";
        for (var i = 0; i < detail.contexts.length; i++) {
            if (detail.contexts[i].indexOf("type=") == 0) {
                type = detail.contexts[i].substring(5);
                break;
            }
        }

        if (type == "home") {
            return "Home";
        } else if (type == "work") {
            return "Work";
        } else if (type == "internet") {
            return "Mobile";
        } else {
            return "Other";
        }
    }

    return "";
}

function setDetailSubType(detail, newSubType) {
    if (!detail) {
        return;
    }

    /* Phone numbers have a special field for the subType */
    if (detail.definitionName == "PhoneNumber") {
        if (newSubType == "Home") {
            detail.subTypes = [ "home", "voice" ];
        } else if (newSubType == "Work") {
            detail.subTypes = [ "work", "voice" ];
        } else if (newSubType == "Work Fax") {
            detail.subTypes = [ "work", "fax" ];
        } else if (newSubType == "Home Fax") {
            detail.subTypes = [ "home", "fax" ];
        } else if (newSubType == "Mobile") {
            detail.subTypes = [ "cell" ];
        } else if (newSubType == "Pager") {
            detail.subTypes = [ "pager" ];
        } else {
            detail.subTypes = [ "other" ];
        }
    } else if (detail.definitionName == "OnlineAccount") {
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
            detail.protocol = PROTOCOL_TYPE_QQ;
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
    } else if (detail.definitionName == "Address") {
        if (newSubType == ADDRESS_LABEL_HOME) {
            detail.subTypes = [ ADDRESS_TYPE_HOME ];
        } else if (newSubType == ADDRESS_LABEL_WORK) {
            detail.subTypes = [ ADDRESS_TYPE_WORK ];
        } else {
            detail.subTypes = [ ADDRESS_TYPE_OTHER ];
        }
    } else {
        var types = ""
        if (newSubType == "Home") {
            types = "home";
        } else if (newSubType == "Work") {
            types = "work";
        } else if (newSubType == "Mobile") {
            types = "internet";
        } else {
            types = "other";
        }

        // We need a copy because QML list properties can't
        // be directly modified, they need to be reassigned a modified copy.
        var contexts = detail.contexts;
        for (var i = 0; i < contexts.length; i++) {
            if (contexts[i].indexOf("type=") == 0) {
                // Modify the first type that we find, since it's the same
                // thing that we did to retrieve the type
                contexts[i] = "type=" + types;
                detail.contexts = contexts;
                return;
            }
        }
        contexts.push("type=" + types);
        detail.contexts = contexts;
    }
}

