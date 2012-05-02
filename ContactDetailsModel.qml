// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

ListModel {
    ListElement {
        section: "Phone"
        type: "Mobile"
        value: "+1 234 567"
    }
    ListElement {
        section: "Phone"
        type: "Home"
        value: "+1 234 567"
    }
    ListElement {
        section: "Phone"
        type: "Work"
        value: "+1 234 567"
    }
    ListElement {
        section: "Email"
        type: "Work"
        value: "someone@somecompany.com"
    }
    ListElement {
        section: "Email"
        type: "Personal"
        value: "someone@somewhere.com"
    }
    ListElement {
        section: "IM"
        type: "Skype"
        value: "someone_using_skype"
    }
    ListElement {
        section: "IM"
        type: "Jabber"
        value: "someone@jabber.org"
    }
    ListElement {
        section: "Address"
        type: "Home"
        value: "123 Some street\nSome Place\n1245678\nSome Country"
    }
}
