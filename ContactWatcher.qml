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
    property variant contact: null
    property string phoneNumber 
    property string customId
    property bool __unknownContact: false

    Component.onCompleted: __checkContact()

    function __checkContact() {
        if (customId && customId != "") {
            contact = contactModel.contactFromCustomId(customId);
            return;
        }
        // try to fill the customId and avoid future queries.
        // in some cases only phoneNumber is set, but this contact
        // has a customId, so in order to avoid calling contactFromPhoneNumber()
        // many times, we cache the customId and wait for this contact to
        // appear in the model.
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
        if (phoneNumber && !__unknownContact) {
            contact = contactModel.contactFromPhoneNumber(phoneNumber);
        }
    }

    function __checkContactAdded(newContact) {
        // check if we hold an instance of a contact already
        if (contact || __unknownContact) {
            return;
        }
        
        if (customId && customId != "" && newContact.customId == customId) {
            contact = newContact;
            return;
        }
        __checkContact()
    }
 
    function __checkContactRemoved(removedCustomId) {
        // check if we hold an instance of a contact already
        if (!contact) {
            return;
        }

        // check if we got removed
        if (customId == removedCustomId) {
            contact = null
            return;
        }
    }
   
    Connections {
        target: contactModel
        onContactAdded: __checkContactAdded(contact)
        onContactRemoved: __checkContactRemoved(customId)
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
