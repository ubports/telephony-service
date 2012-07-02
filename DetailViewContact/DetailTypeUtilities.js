.pragma library

var phoneSubTypes = [ "Home", "Mobile", "Work", "Work Fax", "Home Fax", "Pager", "Other" ];
var emailSubTypes = [ "Work", "Home", "Mobile", "Other" ];
var postalAddressSubTypes = [ "Work", "Home", "Other" ];
var IMSubTypes = [ "AIM", "Windows Live", "Yahoo", "Skype", "QQ", "Google Talk", "ICQ", "Jabber" ];

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
        } else if (detail.subTypes.indexOf("pager") > -1) {
            return "Pager";
        }

        return "Other";
    } else if (detail.definitionName == "OnlineAccount") {
        // TODO: online account information is empty
        return "Other";
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
        // TODO: fix online account information
        return;
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

