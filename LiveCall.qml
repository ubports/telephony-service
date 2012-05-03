import QtQuick 1.0
import "fontUtils.js" as Font

Rectangle {
    id: liveCall
    color: "#ebebeb"

    property variant callStarted: null
    property variant contact: null

    function startCall(contact) {
        liveCall.contact = contact
        callStarted = new Date();
        callTicker.start();
    }

    function endCall() {
        callTicker.stop();
        callStarted = null;
        telephony.endCall();
    }

    Component.onCompleted: startCall(contacts.get(5))

    Image {
        id: picture
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 22
        anchors.top: parent.top

        source: contact ? "dummydata/" + contact.photo : ""
        height: 142
        width: 142
        fillMode: Image.PreserveAspectFit
    }

    TextCustom {
        id: name
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 26
        anchors.top: picture.bottom
        text: contact ? contact.displayName : ""
        fontSize: "xx-large"
    }

    TextCustom {
        id: number
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 8
        anchors.top: name.bottom
        text: contact ? contact.phone : ""
        fontSize: "x-large"
    }

    TextCustom {
        id: location
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 8
        anchors.top: number.bottom
        text: contact ? contact.location : ""
        fontSize: "x-large"
    }

    Timer {
        id: callTicker
        interval: 1000
        repeat: true
        onTriggered: if (callStarted != null) { callDuration.time = (new Date() - callStarted) / 1000 }
    }

    StopWatch {
        id: callDuration
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 12
        anchors.top: location.bottom
    }

    Grid {
        id: mainButtons
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 12
        anchors.top: callDuration.bottom
        columns: 2
        rows: 2
        spacing: 1

        IconButton {
            width: 90
            height: 45
            color: "#797979"
            verticalMargin: 10
            radius: 0
            icon: "assets/icon_keypad_white.png"
        }

        IconButton {
            width: 90
            height: 45
            color: "#797979"
            verticalMargin: 10
            radius: 0
            icon: "assets/icon_speaker_white.png"
        }

        IconButton {
            width: 90
            height: 45
            color: "#797979"
            verticalMargin: 10
            radius: 0
            icon: "assets/icon_pause_white.png"
        }

        IconButton {
            width: 90
            height: 45
            color: "#797979"
            verticalMargin: 10
            radius: 0
            icon: "assets/icon_mute_white.png"
        }
    }

    IconButton {
        id: hangupButton
        anchors.topMargin: 12
        anchors.top: mainButtons.bottom
        anchors.left: mainButtons.left
        verticalMargin: 10

        icon: "assets/icon_hangup_white.png"
        width: 117
        height: 38
        color: "#ef7575"
        borderColor: "#f40000"
        borderWidth: 2

        onClicked: endCall()
    }

    IconButton {
        id: addToContactsButton
        anchors.topMargin: 12
        anchors.top: mainButtons.bottom
        anchors.right: mainButtons.right
        verticalMargin: 10

        icon: "assets/icon_add_call.png"
        width: 57
        height: 38
        color: "#797979"
        borderColor: "white"
        borderWidth: 2
    }
}
