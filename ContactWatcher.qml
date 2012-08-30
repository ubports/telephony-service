import QtQuick 1.1
import QtMobility.contacts 1.1
import TelephonyApp 0.1

/*
 * ContactWatcher in an element used to track changes to a specific
 * contact, based on its contactId or phone number.
 * Pieces of code interested in a specific contact should create
 * an instance of ContactWatcher and set either "phoneNumber" or "contactId".
 * If the contact is not available yet, this element will track the
 * contacts model events and populate the local "contact" property
 * when it becomes available.
 */

Item {
    property variant contact: null
    property string phoneNumber 
    property string contactId
    property bool __unknownContact: false

    Component.onCompleted: __checkContact()

    function __checkContact() {
        if (contactId && contactId != "") {
            contact = contactModel.contactFromId(contactId);
            return;
        }
        // try to fill the contactId and avoid future queries.
        // in some cases only phoneNumber is set, but this contact
        // has a contactId, so in order to avoid calling contactFromPhoneNumber()
        // many times, we cache the contactId and wait for this contact to
        // appear in the model.
        if (phoneNumber && (!contactId || contactId == "") && !__unknownContact) {
            contact = contactModel.contactFromPhoneNumber(phoneNumber);
            if (contact) {
                contactId = contact.id
            }

            if(contactId && contactId != "") {
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
        
        if (contactId && contactId != "" && newContact.id == contactId) {
            contact = newContact;
            return;
        }
        __checkContact()
    }
 
    function __checkContactRemoved(removedContactId) {
        // check if we hold an instance of a contact already
        if (!contact) {
            return;
        }

        // check if we got removed
        if (contactId == removedContactId) {
            contact = null
            return;
        }
    }
   
    Connections {
        target: contactModel
        onContactAdded: __checkContactAdded(contact)
        onContactRemoved: __checkContactRemoved(contactId)
    }

    onPhoneNumberChanged: {
        contactId = ""; 
        __unknownContact = false; 
        __checkContact();
    }

    onContactIdChanged: {
        __unknownContact = false; 
        __checkContact();
    }
}
