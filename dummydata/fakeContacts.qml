/*
 * Copyright 2012 Canonical Ltd.
 *
 * This file is part of telephony-app.
 *
 * telephony-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0

ListModel {
    function fromId(id) {
        return get(id)
    }

    // mimic telepathy contacts API
    Component.onCompleted: {
        var contact
        for(var i = 0; i < count; i++) {
            contact = get(i)
            setProperty(i, "avatar", "../dummydata/" + contact.avatar)
            setProperty(i, "decoration", contact.avatar)
            setProperty(i, "emailAddress", {"emailAddress": contact.email})
            setProperty(i, "phoneNumber", {"number": contact.phone})
            setProperty(i, "phoneNumbers", [{"number": contact.phone, "subTypes": "Mobile"}])
            setProperty(i, "emails", [{"emailAddress": contact.email}])
            setProperty(i, "onlineAccounts", [{"accountUri": "someone@jabber.org", "serviceProvider": "Jabber"}])
            setProperty(i, "addresses", [{"street": "123 Some street", "city": "Some Place", "state": "Some State", "country": "Some Country"}])
            setProperty(i, "name", {"firstName": contact.firstName, "lastName": contact.lastName})
            setProperty(i, "displayLabel", contact.firstName + " " + contact.lastName)
            setProperty(i, "display", contact.displayLabel)
            setProperty(i, "contact", contact)
        }
    }

    ListElement {
        firstName: "Allison"
        lastName: "Reeves"
        email: "a.reeves@mail.com"
        phone: "555-434-6888"
        phoneType: "Mobile"
        avatar: "allisonreeves.jpg"
        favourite: true
        location: "New York"
        presence: 1
    }
    ListElement {
        firstName: "Frank"
        lastName: "Johnson"
        email: "frank.johnson@mail.com"
        phone: "555-224-5532"
        phoneType: "Mobile"
        avatar: ""
        favourite: false
        location: "London"
        presence: 1
    }
    ListElement {
        firstName: "George"
        lastName: "Still"
        email: "georgestill@mail.com"
        phone: "+34 555-334-6545"
        phoneType: "Russia"
        avatar: "georgestill.jpg"
        favourite: true
        location: "San Francisco"
        presence: 1
    }
    ListElement {
        firstName: "Rachel"
        lastName: "Jones"
        email: "rach.jones@mail.com"
        phone: "+1 555-346-7657"
        phoneType: "Work"
        avatar: "racheljones.jpg"
        favourite: true
        location: "San Francisco"
        presence: 1
    }
    ListElement {
        firstName: "Steve"
        lastName: "Jackson"
        email: "steve.capone@mail.com"
        phone: "+55 555-444-333-312-121"
        phoneType: "Work"
        avatar: "stevejackson.jpg"
        favourite: false
        location: "Oakland"
        presence: 1
    }
    ListElement {
        firstName: "Tina"
        lastName: "Gray"
        email: "t.gray@mail.com"
        phone: "555-434-6543"
        phoneType: "Brazil"
        avatar: "tinagray.jpg"
        favourite: false
        location: "Barcelona"
        presence: 1
    }
}
