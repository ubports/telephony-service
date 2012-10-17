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
