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
    // Prepend avatar with "dummydata" path
    Component.onCompleted: {
        for(var i = 0; i < count; i++) {
            var message = get(i)
            var dateTime = new Date(message.timestamp)
            setProperty(i, "avatar", "../dummydata/" + message.avatar)
            setProperty(i, "timestamp", dateTime)
        }
    }

    ListElement {
        avatar: "allisonreeves.jpg"
        contactAlias: "Allison Reeves"
        message: "Hey, how are you doing? Wanna have fun tonight? xx"
        timestamp: "June 13, 2012 11:13:32"
        phoneNumber: "555-434-6888"
        unreadCount: 0
    }
    ListElement {
        avatar: ""
        contactAlias: "Frank Johnson"
        message: "I am an engineer too, bugger off!"
        timestamp: "June 10, 2012 22:10:18"
        phoneNumber: "555-224-5532"
        unreadCount: 2
    }
    ListElement {
        avatar: "racheljones.jpg"
        contactAlias: "Rachel Jones"
        message: "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
        timestamp: "May 28, 2012 16:28:21"
        phoneNumber: "+1 555-346-7657"
        unreadCount: 0
    }
    ListElement {
        avatar: "racheljones.jpg"
        contactAlias: "Rachel Jones"
        message: "I am an engineer too, bugger off!"
        timestamp: "May 28, 2012 15:13:33"
        phoneNumber: "+1 555-346-7657"
        unreadCount: 1
    }
    ListElement {
        avatar: "tinagray.jpg"
        contactAlias: "Tina Gray"
        message: "bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla"
        timestamp: "May 27, 2012 10:00:54"
        phoneNumber: "555-434-6543"
        unreadCount: 0
    }
}
