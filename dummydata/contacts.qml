// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

ListModel {    
    function fromId(id) {
        return get(id)
    }

    ListElement {
        displayName: "Allison Reeves"
        email: "a.reeves@mail.com"
        phone: "555-434-6888"
        phoneType: "Mobile"
        photo: "allisonreeves.jpg"
        favourite: true
        location: "New York"
    }
    ListElement {
        displayName: "Frank Johnson"
        email: "frank.johnson@mail.com"
        phone: "555-224-5532"
        phoneType: "Mobile"
        photo: "frankjohnson.jpg"
        favourite: false
        location: "London"
    }
    ListElement {
        displayName: "George Still"
        email: "georgestill@mail.com"
        phone: "+34 555-334-6545"
        phoneType: "Russia"
        photo: "georgestill.jpg"
        favourite: true
        location: "San Francisco"
    }
    ListElement {
        displayName: "Rachel Jones"
        email: "rach.jones@mail.com"
        phone: "+1 555-346-7657"
        phoneType: "Work"
        photo: "racheljones.jpg"
        favourite: true
        location: "San Francisco"
    }
    ListElement {
        displayName: "Steve Jackson"
        email: "steve.capone@mail.com"
        phone: "+55 555-444-333-312-121"
        phoneType: "Work"
        photo: "stevejackson.jpg"
        favourite: false
        location: "Oakland"
    }
    ListElement {
        displayName: "Tina Gray"
        email: "t.gray@mail.com"
        phone: "555-434-6543"
        phoneType: "Brazil"
        photo: "tinagray.jpg"
        favourite: false
        location: "Barcelona"
    }
}
