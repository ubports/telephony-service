import QtQuick 1.0
import "../Widgets"
import "../DetailViewKeypad"
import "../"

TelephonyPage {
    id: liveCall

    property string viewName: "livecall"
    property alias contact: contactWatcher.contact
    property QtObject call: callManager.foregroundCall
    property alias number: contactWatcher.phoneNumber
    property bool onHold: call ? call.held : false
    property bool isSpeaker: call ? call.speaker : false
    property bool isMuted: call ? call.muted : false
    property bool isDtmf: false

    function endCall() {
        if (call) {
            call.endCall();
        }
    }

    ContactWatcher {
        id: contactWatcher
        phoneNumber: call ? call.phoneNumber : ""
    }

    Connections {
        target: callManager
        onCallEnded: {
            if (!callManager.hasCalls) {
                telephony.endCall();
            }
        }
    }

    Rectangle {
        id: background

        anchors.fill: parent
        color: "#3a3c41"
    }

    BackgroundCall {
        id: backgroundCall

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        call: callManager.backgroundCall
        visible: callManager.hasBackgroundCall
    }

    Item {
        id: container

        anchors.top: backgroundCall.visible ? backgroundCall.bottom : parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        Item {
            id: body

            anchors.centerIn: parent
            width: childrenRect.width
            height: childrenRect.height

            Image {
                id: picture

                anchors.horizontalCenter: parent.horizontalCenter
                width: 140
                height: 140
                sourceSize.width: width
                fillMode: Image.PreserveAspectFit
                source: (contact && contact.avatar != "") ? contact.avatar : "../assets/avatar_incall_rightpane.png"
                onStatusChanged: if (status == Image.Error) source = "../assets/avatar_incall_rightpane.png"
                asynchronous: true
                smooth: true
            }

            BorderImage {
                id: pictureFrame

                source: "../assets/incall_picture_frame.png"
                anchors.fill: picture
                anchors.topMargin: -1
                anchors.bottomMargin: -2
                anchors.leftMargin: -1
                anchors.rightMargin: -1
                border.left: 5
                border.right: 5
                border.top: 6
                border.bottom: 5
                horizontalTileMode: BorderImage.Stretch
                verticalTileMode: BorderImage.Stretch
            }

            TextCustom {
                id: name

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.topMargin: 15
                anchors.top: pictureFrame.bottom
                text: contact ? contact.displayLabel : "Unknown Contact"
                color: "white"
                style: Text.Sunken
                styleColor: Qt.rgba(0.0, 0.0, 0.0, 0.5)
                fontSize: "large"
            }

            TextCustom {
                id: number

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: name.bottom
                anchors.topMargin: 2
                text: liveCall.number
                color: "#a0a0a2"
                style: Text.Sunken
                styleColor: Qt.rgba(0.0, 0.0, 0.0, 0.5)
                fontSize: "medium"
            }

            TextCustom {
                id: location

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: number.bottom
                anchors.topMargin: 2
                // FIXME: add back geo-location information to the ContactEntry model
                text: contact ? contact.geoLocation.label : ""
                color: "#a0a0a2"
                style: Text.Sunken
                styleColor: Qt.rgba(0.0, 0.0, 0.0, 0.5)
                fontSize: "medium"
            }

            TextCustom {
                id: dialing

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.topMargin: 15
                anchors.top: location.text != "" ? location.bottom : number.bottom

                text: "Dialing"
                color: "#a0a0a2"
                style: Text.Sunken
                styleColor: Qt.rgba(0.0, 0.0, 0.0, 0.5)
                fontSize: "medium"
                opacity: (call && call.dialing) ? 1.0 : 0.0
            }

            StopWatch {
                id: stopWatch
                time: call ? call.elapsedTime : 0

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.topMargin: 15
                anchors.top: location.text != "" ? location.bottom : number.bottom
                opacity: (call && !call.dialing) ? 1.0 : 0.0
            }

            Keypad {
                id: keypad

                visible: false
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: mainButtonsContainer.top
                anchors.bottomMargin: 10
                onKeyPressed: {
                    if (call) {
                        call.sendDTMF(label)
                    }
                }
                z: 1
            }

            Item {
                id: mainButtonsContainer

                anchors.top: stopWatch.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                width: mainButtons.width
                height: mainButtons.height

                BorderImage {
                    id: mainButtonsBackground

                    anchors.fill: parent
                    anchors.topMargin: -2
                    anchors.bottomMargin: -2
                    anchors.leftMargin: -3
                    anchors.rightMargin: -2
                    source: "../assets/incall_keypad_background.png"
                    border {right: 14; left: 14; top: 10; bottom: 10}
                    horizontalTileMode: BorderImage.Stretch
                    verticalTileMode: BorderImage.Stretch
                    smooth: true
                }

                Image {
                    id: mainButtonsSeparators

                    anchors.fill: mainButtonsBackground
                    anchors.topMargin: 1
                    anchors.bottomMargin: 2
                    anchors.leftMargin: 2
                    anchors.rightMargin: 1
                    source: "../assets/livecall_keypad_div_tile.png"
                    fillMode: Image.Tile
                }

                Grid {
                    id: mainButtons

                    columns: 2
                    rows: 2

                    LiveCallKeypadButton {
                        corner: Qt.TopLeftCorner
                        iconSource: selected ? "../assets/incall_keypad_dialler_selected.png" : "../assets/incall_keypad_dialler_unselected.png"
                        selected: liveCall.isDtmf
                        onClicked: {
                            liveCall.isDtmf = !liveCall.isDtmf
                            keypad.visible = liveCall.isDtmf
                        }
                    }

                    LiveCallKeypadButton {
                        corner: Qt.TopRightCorner
                        iconSource: selected ? "../assets/incall_keypad_speaker_selected.png" : "../assets/incall_keypad_speaker_unselected.png"
                        selected: liveCall.isSpeaker
                        onClicked: {
                            if (call) {
                                call.speaker = !selected
                            }
                        }
                    }

                    LiveCallKeypadButton {
                        corner: Qt.BottomLeftCorner
                        iconSource: selected ? "../assets/incall_keypad_pause_selected.png" : "../assets/incall_keypad_pause_unselected.png"
                        selected: liveCall.onHold
                        onClicked: {
                            if (call) {
                                call.held = !call.held
                            }
                        }
                    }

                    LiveCallKeypadButton {
                        corner: Qt.BottomRightCorner
                        iconSource: selected ? "../assets/incall_keypad_mute_selected.png" : "../assets/incall_keypad_mute_unselected.png"
                        selected: liveCall.isMuted
                        onClicked: {
                            if (call) {
                                call.muted = !call.muted
                            }
                        }
                    }
                }
            }

            Row {
                anchors.top: mainButtonsContainer.bottom
                anchors.topMargin: 20
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 5

                Button {
                    id: hangupButton

                    iconSource: "../assets/incall_keypad_endcallbutton_icon.png"
                    width: 64
                    color: "#bf400c"
                    darkBorder: true
                    onClicked: endCall()
                }

                Button {
                    id: addToContactsButton

                    iconSource: "../assets/incall_keypad_addcaller_unselected.png"
                    width: 64
                    color: "#666666"
                    darkBorder: true
                }
            }
        }
    }
}
