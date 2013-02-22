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
import Ubuntu.Components 0.1
import "../Widgets" as LocalWidgets
import "../PanelDialer"
import "../"

LocalWidgets.TelephonyPage {
    id: liveCall

    property alias contact: contactWatcher.contact
    property QtObject call: callManager.foregroundCall
    property alias number: contactWatcher.phoneNumber
    property bool onHold: call ? call.held : false
    property bool isSpeaker: call ? call.speaker : false
    property bool isMuted: call ? call.muted : false
    property alias isDtmf: flipable.flipped

    title: call && call.dialing ? "Dialing" : "Duration " + stopWatch.elapsed
    tools: ToolbarActions {
        active: false
        lock: true
    }

    function endCall() {
        if (call) {
            call.endCall();
        }
    }

    LocalWidgets.StopWatch {
        id: stopWatch
        time: call ? call.elapsedTime : 0
        visible: false
    }

    ContactWatcher {
        id: contactWatcher
        phoneNumber: call ? call.phoneNumber : ""
    }

    Connections {
        target: callManager
        onCallEnded: {
            if (!callManager.hasCalls && telephony.liveCall.loaded) {
                var filterProperty = "phoneNumber";
                var filterValue = contactWatcher.phoneNumber;
                if (contactWatcher.contactId != "") {
                    filterProperty = "contactId";
                    filterValue = contactWatcher.contactId;
                }

                telephony.showCommunication(filterProperty, filterValue, "", contactWatcher.contactId, true);
                telephony.endCall();
            }
        }
    }

    BackgroundCall {
        id: backgroundCall

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        call: callManager.backgroundCall
        visible: callManager.hasBackgroundCall
    }

    LocalWidgets.Header {
        id: pageHeader
        text: title
    }

    Flipable {
        id: flipable
        anchors {
            top: pageHeader.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        
        property bool flipped: false
        transform: Rotation {
            id: rotation
            origin.x: flipable.width/2
            origin.y: flipable.height/2
            axis.x: 0; axis.y: 1; axis.z: 0
            angle: 0    // the default angle
        }

        states: State {
            name: "back"
            PropertyChanges { target: rotation; angle: 180 }
            when: flipable.flipped
        }

        // avoid events on the wrong view
        onSideChanged: { 
            front.visible = (side == Flipable.Front);
            back.visible = (side == Flipable.Back);
        }

        transitions: Transition {
            NumberAnimation { target: rotation; property: "angle"; duration: 400 }
        }

        front: Item {
            anchors.top: backgroundCall.visible ? backgroundCall.bottom : parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            Item {
                id: header
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                height: units.gu(11)

                UbuntuShape {
                    id: picture

                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: units.gu(2)
                    width: units.gu(7)
                    height: units.gu(7)
                    image: Image {
                        source: (contact && contact.avatar != "") ? contact.avatar : "../assets/avatar-default.png"
                        fillMode: Image.PreserveAspectCrop
                        // since we don't know if the image is portrait or landscape without actually reading it,
                        // set the sourceSize to be the size we need plus 30% to allow cropping.
                        sourceSize.width: width * 1.3
                        sourceSize.height: height * 1.3
                        asynchronous: true
                    }
                }

                Label {
                    id: name

                    anchors.top: picture.top
                    anchors.left: picture.right
                    anchors.leftMargin: units.gu(2)
                    text: contact ? contact.displayLabel : "Unknown Contact"
                    color: "#a0a0a2"
                    fontSize: "large"
                }

                Label {
                    id: number

                    anchors.top: name.bottom
                    anchors.topMargin: units.dp(2)
                    anchors.left: picture.right
                    anchors.leftMargin: units.gu(2)
                    text: liveCall.number
                    color: "#a0a0a2"
                    fontSize: "medium"
                }
            }

            Image {
                id: divider2

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: header.bottom
                source: "../assets/horizontal_divider.png"
            }


            Item {
                id: body

                anchors.top: divider2.bottom
                anchors.right: parent.right
                anchors.left: parent.left
                height: units.gu(36)
                Grid {
                    id: mainButtonsGrid
                    rows: 2
                    columns: 3
                    spacing: units.dp(6)

                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    width: childrenRect.width
                    height: childrenRect.height

                    LiveCallKeypadButton {
                        iconSource: selected ? "../assets/play.png" : "../assets/pause.png"
                        selected: liveCall.onHold
                        onClicked: {
                            if (call) {
                                call.held = !call.held
                            }
                        }
                    }

                    LiveCallKeypadButton {
                        iconSource: selected ? "../assets/speaker.png" : "../assets/speaker-mute.png"
                        selected: liveCall.isSpeaker
                        onClicked: {
                            if (call) {
                                call.speaker = !selected
                            }
                        }
                    }

                    LiveCallKeypadButton {
                        iconSource: selected ? "../assets/microphone-mute.png" : "../assets/microphone.png"
                        selected: liveCall.isMuted
                        onClicked: {
                            if (call) {
                                call.muted = !call.muted
                            }
                        }
                    }

                    LiveCallKeypadButton {
                        iconSource: "../assets/quick-add.png"
                        selected: false
                        onClicked: {
                        }
                    }

                    LiveCallKeypadButton {
                        iconSource: "../assets/keypad.png"
                        selected: liveCall.isDtmf
                        onClicked: {
                            liveCall.isDtmf = true
                        }
                    }

                    LiveCallKeypadButton {
                        iconSource: "../assets/live_call_contacts.png"
                        selected: false
                        onClicked: {
                        }
                    }
                }
            }

            Item {
                id: footer
                height: units.gu(12)
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right

                Image {
                    id: divider3

                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    source: "../assets/horizontal_divider.png"
                }

                HangupButton {
                    id: hangupButton

                    anchors.top: divider3.bottom
                    anchors.topMargin: units.gu(2)
                    anchors.horizontalCenter: parent.horizontalCenter
                    onClicked: endCall()
                }
            }
        }

        back: FocusScope {
            anchors.fill: parent
            focus: true

            KeypadEntry {
                id: keypadEntry

                anchors.top: parent.top
                anchors.left: keypad.left
                anchors.right: keypad.right
                anchors.leftMargin: units.dp(-2)
                anchors.rightMargin: units.dp(-2)
                placeHolder: liveCall.number
                placeHolderPixelFontSize: units.dp(43)
                focus: true
                input.readOnly: true
            }

            Image {
                id: divider4

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: keypadEntry.bottom
                source: "../assets/dialer_top_number_bg.png"
            }

            Keypad {
                id: keypad

                anchors.top: divider4.bottom
                onKeyPressed: {
                    keypadEntry.value += label
                    if (call) {
                        call.sendDTMF(label)
                    }
                }

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.topMargin: units.dp(10)
            }

            Item {
                id: dialFooter
                height: units.gu(12)
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right

                Image {
                    id: divider5

                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    source: "../assets/horizontal_divider.png"
                }

                HangupButton {
                    id: hangupButton2

                    anchors.top: divider5.bottom
                    anchors.topMargin: units.gu(2)
                    anchors.horizontalCenter: parent.horizontalCenter
                    onClicked: endCall()
                }

                CustomButton {
                    id: backButton
                    objectName: "backButton"
                    anchors.right: hangupButton2.left
                    anchors.verticalCenter: hangupButton2.verticalCenter
                    anchors.rightMargin: units.gu(1)
                    // FIXME: use the right icon
                    icon: "../assets/back.png"
                    iconWidth: units.gu(4)
                    iconHeight: units.gu(4)
                    width: units.gu(7)
                    height: units.gu(7)
                    onClicked: liveCall.isDtmf = false
                }
            }
        }
    }
}
