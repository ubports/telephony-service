import QtQuick 1.0
import "../Widgets"
import "../DetailViewKeypad"

Item {
    id: voicemail

    property string viewName: "voicemail"
    // FIXME: better name that does not sound like a boolean; store it in the StopWatch but also alias it here
    // FIXME: refactor StopWatch, Timer into StopWatch
    property variant contact
    property QtObject call: callManager.foregroundCall
    property string number: callManager.getVoicemailNumber()

    function isVoicemailActive() {
        return call && call.voicemail
    }

    function endCall() {
        if (call) {
            call.endCall();
        }
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

    Item {
        id: container

        anchors.fill: parent

        Item {
            id: body

            anchors.fill: parent
            //width: childrenRect.width
            //height: childrenRect.height

            TextCustom {
                id: name

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.topMargin: 15
                anchors.bottom: number.top
                text: "Voicemail"
                color: "white"
                style: Text.Sunken
                styleColor: Qt.rgba(0.0, 0.0, 0.0, 0.5)
                fontSize: "x-large"
            }

            TextCustom {
                id: number

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: stopWatch.top
                anchors.topMargin: 2
                text: voicemail.number
                color: "#a0a0a2"
                style: Text.Sunken
                styleColor: Qt.rgba(0.0, 0.0, 0.0, 0.5)
                fontSize: "large"
            }

            StopWatch {
                id: stopWatch
                time: call && call.voicemail ? call.elapsedTime : 0

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.topMargin: 15
                anchors.bottom: keypad.top
            }

            Keypad {
                id: keypad

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                onKeyPressed: {
                    if (call) {
                        call.sendDTMF(label)
                    }
                }
            }

            Button {
                id: dialhangupButton

                iconSource: isVoicemailActive() ? "../assets/incall_keypad_endcallbutton_icon.png" : "../assets/call_icon.png"
                anchors.top: keypad.bottom
                anchors.topMargin: 20
                anchors.horizontalCenter: parent.horizontalCenter
                width: 64
                color: isVoicemailActive() ? "#bf400c" : "#37b301"
                onClicked: {
                    if(isVoicemailActive())
                        endCall()
                    else
                        telephony.callNumber(voicemail.number)
                }
            }

        }
    }
}
