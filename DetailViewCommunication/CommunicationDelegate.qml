import QtQuick 2.0
import Ubuntu.Components 0.1

Item {
    id: communicationDelegate
    property string itemType
    property string message: "Test"
    property alias itemIcon: icon.source
    property bool incoming: false
    property variant timestamp
    property bool missed: false
    property variant item

    signal clicked(var mouse)

    anchors.left: parent ? parent.left : undefined
    anchors.right: parent ? parent.right: undefined

    height: bubble.height + units.gu(1.5)

    Component.onCompleted: {
        if (item.newItem) {
            item.newItem = false;
        }
    }

    UbuntuShape {
        id: bubble

        anchors.left: incoming ? icon.right : parent.left
        anchors.leftMargin: incoming ? units.gu(1) : units.gu(2)
        anchors.right: incoming ? parent.right : icon.left
        anchors.rightMargin: incoming ? units.gu(2) : units.gu(1)
        anchors.top: parent.top
        anchors.topMargin: units.gu(0.5)

        height: messageText.height + units.gu(3)
        color: (incoming && itemType != "call") ? "#dd5314" : "#f5f5f5"

        TextCustom {
            id: messageText

            anchors.left: bubble.left
            anchors.top: bubble.top
            anchors.right: bubble.right
            anchors.margins: units.gu(1)
            height: paintedHeight

            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            fontSize: "small"
            color: (incoming && itemType != "call") ? "#ffffff" : "#333333"
            opacity: incoming ? 1 : 0.9
            text: selectMessage()

            function selectMessage() {
                if (communicationDelegate.itemType == "call") {
                    if (missed) {
                        return "Missed call";
                    } else {
                        return "Call";
                    }
                } else {
                    return communicationDelegate.message;
                }
            }
        }
    }

    Rectangle {
        id: arrow

        anchors.horizontalCenter: incoming ? bubble.right : bubble.left
        anchors.verticalCenter: bubble.bottom
        anchors.verticalCenterOffset: units.gu(-2)

        rotation: 45
        width: units.gu(1.4)
        height: units.gu(1.4)

        color: bubble.color
    }

    Image {
        id: icon
        width: units.gu(2)
        height: units.gu(2)

        anchors.left: incoming ? parent.left : undefined
        anchors.leftMargin: units.gu(2)
        anchors.right: incoming ? undefined : parent.right
        anchors.rightMargin: units.gu(2)
        anchors.top: parent.top
        anchors.topMargin: units.gu(1)
    }

    MouseArea {
        anchors.fill: parent
        onClicked: communicationDelegate.clicked(mouse)
    }
}
