/*
 * Copyright 2012 Canonical Ltd.
 *
 * This file is part of telephony-app.
 *
 * telephony-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0

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
