import QtQuick 1.0
import "fontUtils.js" as Font

Rectangle {
    color: "#ebebeb"

    Image {
        id: picture
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 22
        anchors.top: parent.top

        source: "assets/picture_anna.png"
        height: 142
        width: 142
        fillMode: Image.PreserveAspectFit
    }

    Text {
        id: name
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 26
        anchors.top: picture.bottom
        text: "Anna Olson"
        font.pixelSize: Font.sizeToPixels("xx-large")
    }

    Text {
        id: number
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 8
        anchors.top: name.bottom
        text: "+23 453-245-2321"
        font.pixelSize: Font.sizeToPixels("x-large")
    }

    Text {
        id: location
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 8
        anchors.top: number.bottom
        text: "London"
        font.pixelSize: Font.sizeToPixels("x-large")
    }

    StopWatch {
        id: time
        time: "00:54"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 12
        anchors.top: location.bottom
    }

    Grid {
        id: mainButtons
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 12
        anchors.top: time.bottom
        columns: 2
        rows: 2
        spacing: 1
        IconButton { icon: "assets/icon_keypad_white.png" }
        IconButton { icon: "assets/icon_speaker_white.png" }
        IconButton { icon: "assets/icon_pause_white.png" }
        IconButton { icon: "assets/icon_mute_white.png" }
    }

    IconButton {
        id: hangupButton
        anchors.topMargin: 12
        anchors.top: mainButtons.bottom
        anchors.left: mainButtons.left
        radius: 4

        icon: "assets/icon_hangup_white.png"
        width: 117
        height: 38
        color: "#ef7575"
        border.color: "#f40000"
        border.width: 2
    }

    IconButton {
        id: addToContactsButton
        anchors.topMargin: 12
        anchors.top: mainButtons.bottom
        anchors.right: mainButtons.right
        radius: 4

        icon: "assets/icon_add_call.png"
        width: 57
        height: 38
        border.color: "white"
        border.width: 2
    }

//    anchors.bottomMargin: 24

}
