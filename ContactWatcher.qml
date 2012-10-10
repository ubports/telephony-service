import QtQuick 2
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

    Component.onCompleted: __checkContact()

    function __checkContact() {
        if (contactId && contactId != "") {
            contact = contactModel.contactFromId(contactId);
            return;
        }
        // try to fill the contactId. In some cases only phoneNumber 
        // is set, but this contact has a contactId already.
        if (phoneNumber && (!contactId || contactId == "")) {
            contact = contactModel.contactFromPhoneNumber(phoneNumber);
            if (contact) {
                contactId = contact.id
            }

            if(contactId && contactId != "") {
                return;
            } else {
                contact = null;
                return;
            }
        }

        if (phoneNumber) {
            contact = contactModel.contactFromPhoneNumber(phoneNumber);
        }
    }

    function __checkContactAdded(newContact) {
        // check if we hold an instance of a contact already
        if (contact) {
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
        __checkContact();
    }

    onContactIdChanged: {
        __checkContact();
    }
}
