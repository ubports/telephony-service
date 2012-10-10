import QtQuick 2

ListModel {
    // Convert timestamp from string into Date objects
    Component.onCompleted: {
        for(var i = 0; i < count; i++) {
            var entry = get(i)
            var dateTime = new Date(entry.timestamp)
            setProperty(i, "timestamp", dateTime)
        }
    }

    ListElement {
        contactId: 0
        contactAlias: "Allison Reeves"
        phoneType: "Mobile"
        phoneNumber: "555-434-6888"
        avatar: "../dummydata/allisonreeves.jpg"
        incoming: true
        missed: false
        timestamp: "June 13, 2012 11:13:32"
    }
    ListElement {
        contactId: 1
        contactAlias: "Frank Johnson"
        phoneType: "Mobile"
        phoneNumber: "555-434-6888"
        avatar: "../dummydata/frankjohnson.jpg"
        incoming: false
        missed: false
        timestamp: "June 11, 2012 15:31:01"
    }
    ListElement {
        contactId: 1
        contactAlias: "Frank Johnson"
        phoneType: "Mobile"
        phoneNumber: "555-434-6888"
        avatar: "../dummydata/frankjohnson.jpg"
        incoming: true
        missed: true
        timestamp: "June 11, 2012 23:03:35"
    }

}
