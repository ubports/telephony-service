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
    id: messagesModel

    ListElement {
        date: "Tuesday, May 01, 2012"
        message: "testlk jsdlfj salkfj lsjlfkjsaf lskjlsakd jfksjdlfk jsalfdjlsakjfl kaslfjlsajf"
        avatar: ""
        timestamp: "2:05 PM"
        incoming: true
    }
    ListElement {
        date: "Tuesday, May 01, 2012"
        message: "test2 lkjsl fjsaljflsakfk jsdlfajsld jflskjdflkjsal fjlsadjflsjalfj lsjdfljsadlfjlsdjflsajdflsjadfjlkdsjf"
        avatar: ""
        timestamp: "2:08 PM"
        incoming: false
    }
    ListElement {
        date: "Tuesday, May 01, 2012"
        message: "tes t3lkjsdflj salkflkj sldjfsj dflkjsalkfjl sjdfljsaldjfl sadjflsj dfjsljflsajfdljsalfjl"
        avatar: ""
        timestamp: "3:15 PM"
        incoming: true
    }
    ListElement {
        date: "Tuesday, May 01, 2012"
        message: "tes t3lkjsdflj salkflkj sldjfsj dflkjsalkfjl sjdfljsaldjfl sadjflsj dfjsljflsajfdljsalfjl"
        avatar: ""
        timestamp: "3:35 PM"
        incoming: false
    }
    ListElement {
        date: "Wednesday, May 02, 2012"
        message: "tes t3lkjsdflj salkflkj sldjfsj dflkjsalkfjl sjdfljsaldjfl sadjflsj dfjsljflsajfdljsalfjl tes t3lkjsdflj salkflkj sldjfsj dflkjsalkfjl sjdfljsaldjfl sadjflsj dfjsljflsajfdljsalfjl"
        avatar: ""
        timestamp: "10:15 AM"
        incoming: true
    }
    ListElement {
        date: "Wednesday, May 02, 2012"
        message: "tes t3lkjsdflj salkflkj sldjfsj"
        avatar: ""
        timestamp: "11:21 AM"
        incoming: true
    }
    ListElement {
        date: "Thursday, May 03, 2012"
        message: "tes t3lkjsdflj salkflkj sldjfsj dflkjsalkfjl sjdfljsaldjfl sadjflsj dfjsljflsajfdljsalfjl tes t3lkjsdflj salkflkj sldjfsj dflkjsalkfjl sjdfljsaldjfl sadjflsj dfjsljflsajfdljsalfjl"
        avatar: ""
        timestamp: "10:15 PM"
        incoming: false

    }
    ListElement {
        date: "Thursday, May 03, 2012"
        message: "tes t3lkjsdflj salkflkj sldjfsj"
        avatar: ""
        timestamp: "11:21 PM"
        incoming: true
    }
    ListElement {
        date: "Thursday, May 03, 2012"
        message: ""
        avatar: "../dummydata/fake_mms.jpg"
        timestamp: "11:21 PM"
        incoming: true
    }
    ListElement {
        date: "Thursday, May 05, 2012"
        message: ""
        avatar: "../dummydata/fake_mms_vertical.jpg"
        timestamp: "11:21 PM"
        incoming: false
    }
 }
