import QtQuick 1.1
import "../fontUtils.js" as FontUtils

Item {
    property string fontColor: "white"
    property int fontSize: FontUtils.sizeToPixels("medium")

    property int margins: 5
    property string filter: null

    height: visible ? 50 : 0
    width: 200
    visible: {
        if (!filter) {
            return false
        }

        var lowerCaseContactName = contactName.text.toLowerCase()
        var lowerCaseFilter = filter.toLowerCase()
        if (lowerCaseContactName.indexOf(lowerCaseFilter) >= 0) {
            return true
        } else if (phone.indexOf(lowerCaseFilter) >= 0) {
                return true
        }
        return false
    }
    anchors.margins: 30

    Rectangle {
        anchors.fill: parent
        color: "gray"
    }

    Rectangle {
        id: div
        color: fontColor
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 1
    }

    Text {
        id: contactName
        text: displayName
        font.bold: true
        font.pixelSize: fontSize
        color: fontColor
        anchors.left: parent.left
        anchors.top: div.bottom
        anchors.margins: margins
    }

    Text {
        text: phoneType
        color: fontColor
        font.pixelSize: fontSize
        anchors.right: parent.right
        anchors.top: div.bottom
        anchors.margins: margins
    }

    Text {
        text: phone
        color: "dark gray"
        font.pixelSize: fontSize
        anchors.left: parent.left
        anchors.top: contactName.bottom
        anchors.margins: margins
    }
}
