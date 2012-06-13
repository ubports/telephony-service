import QtQuick 1.1

ListModel {
    ListElement {
        contactId: 0
        contactAlias: "Allison Reeves"
        phoneType: "Mobile"
        phoneNumber: "555-434-6888"
        avatar: "../dummydata/allisonreeves.jpg"
        incoming: true
        missed: false
        timestamp: "10:23 AM"
    }
    ListElement {
        contactId: 1
        contactAlias: "Frank Johnson"
        phoneType: "Mobile"
        phoneNumber: "555-434-6888"
        avatar: "../dummydata/frankjohnson.jpg"
        incoming: false
        missed: false
        timestamp: "9:59 AM"
    }
    ListElement {
        contactId: 1
        contactAlias: "Frank Johnson"
        phoneType: "Mobile"
        phoneNumber: "555-434-6888"
        avatar: "../dummydata/frankjohnson.jpg"
        incoming: true
        missed: true
        timestamp: "9:59 AM"
    }

}
