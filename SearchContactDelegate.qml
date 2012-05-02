import QtQuick 1.1

Item {
    property string fontColor: "white"
    property int margins: 5
    property string filter: null

    height: visible ? childrenRect.height : 0
    width: 200
    visible: filter ? (contactName.text.toLowerCase().indexOf(filter.toLowerCase()) >= 0) : false

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
        color: fontColor
        anchors.left: parent.left
        anchors.top: div.bottom
        anchors.margins: margins
    }

    Text {
        text: phoneType
        color: fontColor
        anchors.right: parent.right
        anchors.top: div.bottom
        anchors.margins: margins
    }

    Text {
        text: phone
        color: fontColor
        anchors.left: parent.left
        anchors.top: contactName.bottom
        anchors.margins: margins
    }
}
