.pragma library

var phoneSubTypes = ["Home", "Mobile", "Work", "Work Fax", "Home Fax", "Pager", "Other"];
var emailSubTypes = [ "Work", "Home", "Mobile", "Other" ];
var postalAddressSubTypes = [ "Work", "Home", "Other" ];
var IMSubTypes = [ "AIM", "Windows Live", "Yahoo", "Skype", "QQ", "Google Talk", "ICQ", "Jabber" ];

var supportedTypes = [
            {
                name: "Phone",
                delegateSource: "TextContactDetailsDelegate.qml",
                items: "phoneNumbers",
                newItemText: "Add a phone number",
                newItemType: "PhoneNumber",
                actionIcon: "../assets/icon_message_grey.png",
                displayField: "number",
                subTypes: phoneSubTypes
            },
            {
                name: "Email",
                delegateSource: "TextContactDetailsDelegate.qml",
                items: "emails",
                newItemText: "Add an email address",
                newItemType: "EmailAddress",
                actionIcon: "../assets/icon_envelope_grey.png",
                displayField: "emailAddress",
                subTypes: emailSubTypes
            },
            {
                name: "Address",
                delegateSource: "TextContactDetailsDelegate.qml",
                items: "addresses",
                newItemText: "Add a postal address",
                newItemType: "Address",
                actionIcon: "../assets/icon_address.png",
                delegateSource: "AddressContactDetailsDelegate.qml",
                subTypes: postalAddressSubTypes
            },
            {
                name: "IM",
                delegateSource: "TextContactDetailsDelegate.qml",
                items: "onlineAccounts",
                newItemText: "Add an online account",
                displayField: "accountUri",
                newItemType: "OnlineAccount",
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
   /* Phone numbers have a special field for the subType */
   if (detail.toString().indexOf("QDeclarativeContactPhoneNumber") == 0)
        return (detail.subTypes.length > 0) ? detail.subTypes[0] : ""

    // The backend supports multiple types but we can just handle one,
    // so let's pick just the first
    if (detail) {
        for (var i = 0; i < detail.contexts.length; i++) {
            if (detail.contexts[i].indexOf("type=") == 0) {
                return detail.contexts[i].substring(5);
            }
        }
    }
    return "";
}

function setDetailSubType(detail, newSubType) {
    if (detail) {
        if (detail.toString().indexOf("QDeclarativeContactPhoneNumber") == 0) {
            detail.subTypes = [newSubType];
            return;
        }

       // We need a copy because QML list properties can't
        // be directly modified, they need to be reassigned a modified copy.
        var contexts = detail.contexts;
        for (var i = 0; i < contexts.length; i++) {
            if (contexts[i].indexOf("type=") == 0) {
                // Modify the first type that we find, since it's the same
                // thing that we did to retrieve the type
                contexts[i] = "type=" + newSubType.toLowerCase()
                detail.contexts = contexts
                return;
            }
        }
        contexts.push("type=" + newSubType.toLowerCase())
        detail.contexts = contexts
    }
    return "";
}

