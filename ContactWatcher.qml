import QtQuick 1.1
import QtMobility.contacts 1.1
import TelephonyApp 0.1

/*
 * ContactWatcher in an element used to track changes to a specific
 * contact, based on its customId or phone number.
 * Pieces of code interested in a specific contact should create
 * an instance of ContactWatcher and set either "phoneNumber" or "customId".
 * If the contact is not available yet, this element will track the
 * contacts model events and populate the local "contact" property
 * when it becomes available.
 */

Item {
    property variant contact
    property string phoneNumber 
    property string customId
    property bool __unknownContact: false

    Component.onCompleted: __checkContact()

    function __checkContact() {
        if(customId && customId != "") {
            contact = contactModel.contactFromCustomId(customId);
            return;
        }
        // try to fill the customId and avoid future queries
        if (phoneNumber && (!customId || customId == "") && !__unknownContact) {
            customId = contactModel.customIdFromPhoneNumber(phoneNumber);
            if(customId && customId != "") {
                return;
            } else {
                __unknownContact = true;
                contact = null;
                return;
            }
        }
        // if this contact does not exist on database, 
        // dont waste time asking the backend about it.
        if(phoneNumber && !__unknownContact) {
            contact = contactModel.contactFromPhoneNumber(phoneNumber);
        }
    }
    
    Connections {
        target: contactModel
        onContactAdded: __checkContact()
        onContactRemoved: __checkContact()
    }

    onPhoneNumberChanged: {
        customId = ""; 
        __unknownContact = false; 
        __checkContact();
    }

    onCustomIdChanged: {
        __unknownContact = false; 
        __checkContact();
    }
}
