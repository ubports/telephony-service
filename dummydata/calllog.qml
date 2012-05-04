import QtQuick 1.1

ListModel {
    ListElement {
        displayName: "Allison Reeves"
        phoneType: "Mobile"
        phone: "555-434-6888"
        photo: "allisonreeves.jpg"
        direction: "incoming"
        missed: false
        date: "10:23 AM"
    }
    ListElement {
        displayName: "Frank Johnson"
        phoneType: "Mobile"
        phone: "555-434-6888"
        photo: "frankjohnson.jpg"
        direction: "outgoing"
        missed: false
        date: "9:59 AM"
    }
    ListElement {
        displayName: "Frank Johnson"
        phoneType: "Mobile"
        phone: "555-434-6888"
        photo: "frankjohnson.jpg"
        direction: "incoming"
        missed: true
        date: "9:59 AM"
    }

}
