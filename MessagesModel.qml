// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

ListModel {
     id: messagesModel
     ListElement {
         section: "Tuesday, May 01, 2012"
         message: "testlk jsdlfj salkfj lsjlfkjsaf lskjlsakd jfksjdlfk jsalfdjlsakjfl kaslfjlsajf"
         timeStamp: "2:05 PM"
         outgoing: true
     }
     ListElement {
         section: "Tuesday, May 01, 2012"
         message: "test2 lkjsl fjsaljflsakfk jsdlfajsld jflskjdflkjsal fjlsadjflsjalfj lsjdfljsadlfjlsdjflsajdflsjadfjlkdsjf"
         timeStamp: "2:08 PM"
         outgoing: false
     }
     ListElement {
        section: "Tuesday, May 01, 2012"
        message: "tes t3lkjsdflj salkflkj sldjfsj dflkjsalkfjl sjdfljsaldjfl sadjflsj dfjsljflsajfdljsalfjl"
        timeStamp: "3:15 PM"
        outgoing: true
     }
     ListElement {
         section: "Tuesday, May 01, 2012"
         message: "tes t3lkjsdflj salkflkj sldjfsj dflkjsalkfjl sjdfljsaldjfl sadjflsj dfjsljflsajfdljsalfjl"
         timeStamp: "3:35 PM"
         outgoing: false
    }
     ListElement {
         section: "Monday, May 02, 2012"
         message: "tes t3lkjsdflj salkflkj sldjfsj dflkjsalkfjl sjdfljsaldjfl sadjflsj dfjsljflsajfdljsalfjl tes t3lkjsdflj salkflkj sldjfsj dflkjsalkfjl sjdfljsaldjfl sadjflsj dfjsljflsajfdljsalfjl"
         timeStamp: "10:15 AM"
         outgoing: true

     }
     ListElement {
         section: "Monday, May 02, 2012"
        message: "tes t3lkjsdflj salkflkj sldjfsj"
        timeStamp: "11:21 AM"
        outgoing: true
     }

 }
