import QtQuick 1.1
import "../Widgets"
import "../fontUtils.js" as Font

Rectangle {
    property alias image: icon.source
    property alias text: textCustom.text
    property string preText: ""
    property string postText: ""
    color: "#ebebeb"

    Image {
        id: icon
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -40

        fillMode: Image.PreserveAspectFit
    }

    TextCustom {
        id: preTextCustom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 4
        anchors.top: icon.bottom

        font.pixelSize: Font.sizeToPixels("large")
        text: preText
        color: "#a9a9a9"
    }

    TextCustom {
        id: textCustom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: preTextCustom.bottom

        font.pixelSize: Font.sizeToPixels("x-large")
        color: "#a9a9a9"
    }

    TextCustom {
        id: postTextCustom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: textCustom.bottom

        font.pixelSize: Font.sizeToPixels("large")
        text: postText
        color: "#a9a9a9"
    }
}
