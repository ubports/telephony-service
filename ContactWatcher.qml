import QtQuick 1.1
import QtMobility.contacts 1.1
import TelephonyApp 0.1

Item {
    property variant contact
    property string number
    property string contactId
    property bool unknownContact: false

    Component.onCompleted: __checkContact()

    function __checkContact() {
        if(contactId && contactId != "") {
            contact = contactModel.contactFromCustomId(contactId);
            return;
        }
        // try to fill the contactId and avoid future queries
        if (number && (!contactId || contactId =="") && unknownContact == false) {
            contactId = contactModel.customIdFromPhoneNumber(number);
            if(contactId && contactId != "") {
                return;
            } else {
                unknownContact = true;
                contact = null;
                return;
            }
        }
        // if this contact does not exist on database, 
        // dont waste time asking the backend about it.
        if(number && unknownContact == false) {
            contact = contactModel.contactFromPhoneNumber(number);
        }
    }
 
    Connections {
        target: contactModel
        onContactAdded: __checkContact()
        onContactRemoved: __checkContact()
    }

    onNumberChanged: {
        contactId = ""; 
        unknownContact = false; 
        __checkContact();
    }

    onContactIdChanged: {
        unknownContact = false; 
        __checkContact();
    }
}
