import QtQuick 1.1

ListModel {
    ListElement {
        section: "Phone"
        type: "Mobile"
        value: "+1 234 567"
        actionIcon: "assets/icon_message_grey.png"
    }
    ListElement {
        section: "Phone"
        type: "Home"
        value: "+1 234 567"
        actionIcon: "assets/icon_message_grey.png"
    }
    ListElement {
        section: "Phone"
        type: "Work"
        value: "+1 234 567"
        actionIcon: "assets/icon_message_grey.png"
    }
    ListElement {
        section: "Email"
        type: "Work"
        value: "someone@somecompany.com"
        actionIcon: "assets/icon_envelope_grey.png"
    }
    ListElement {
        section: "Email"
        type: "Personal"
        value: "someone@somewhere.com"
        actionIcon: "assets/icon_envelope_grey.png"
    }
    ListElement {
        section: "IM"
        type: "Skype"
        value: "someone_using_skype"
        actionIcon: ""
    }
    ListElement {
        section: "IM"
        type: "Jabber"
        value: "someone@jabber.org"
        actionIcon: ""
    }
    ListElement {
        section: "Address"
        type: "Home"
        value: "123 Some street\nSome Place\n1245678\nSome Country"
        actionIcon: "assets/icon_address.png"
    }
}
