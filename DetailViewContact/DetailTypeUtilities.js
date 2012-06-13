.pragma library

var supportedTypes = [
            {
                name: "Phone",
                delegateSource: "TextContactDetailsDelegate.qml",
                items: "phoneNumbers",
                newItemText: "Add a phone number",
                newItemType: "PhoneNumber",
                actionIcon: "../assets/icon_message_grey.png",
                displayField: "number"
            },
            {
                name: "Email",
                delegateSource: "TextContactDetailsDelegate.qml",
                items: "emails",
                newItemText: "Add an email address",
                newItemType: "EmailAddress",
                actionIcon: "../assets/icon_envelope_grey.png",
                displayField: "emailAddress" },
            {
                name: "Address",
                delegateSource: "TextContactDetailsDelegate.qml",
                items: "addresses",
                newItemText: "Add a postal address",
                newItemType: "Address",
                actionIcon: "../assets/icon_address.png",
                delegateSource: "AddressContactDetailsDelegate.qml"
            },
            {
                name: "IM",
                delegateSource: "TextContactDetailsDelegate.qml",
                items: "onlineAccounts",
                newItemText: "Add an online account",
                displayField: "accountUri",
                newItemType: "OnlineAccount"
            }
        ];

function getTypesWithNoItems(contact) {
    var result = [];
    for (var i = 0; i < supportedTypes.length; i++) {
        var currentType = supportedTypes[i];
        var detailsList = contact[currentType.items];
        if (detailsList && detailsList.length == 0) {
            result.push(currentType);
        }
    }
    return result;
}

