import QtQuick 1.1
import "../fontUtils.js" as Font

Rectangle {
    id: callItem
    height: 64
    width: parent.width

    signal clicked(int index)

    Rectangle {
        height: 1
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: "black"
    }

    Image {
        id: photoItem
        // FIXME: move the dummydata to the model
        source: "../dummydata/" + photo
        width: 56
        height: width
        anchors.left: parent.left
        anchors.leftMargin: 1
        anchors.verticalCenter: parent.verticalCenter
    }
    // FIXME: TextCustom
    Text {
        id: displayNameItem
        anchors.top: parent.top
        anchors.topMargin: 1
        anchors.left: photoItem.right
        anchors.right: directionItem.left
        anchors.leftMargin: 2
        font.pixelSize: Font.sizeToPixels("medium")
        text: displayName
    }

    Text {
        id: phoneTypeItem
        anchors.top: displayNameItem.bottom
        anchors.topMargin: 2
        anchors.left: photoItem.right
        anchors.leftMargin: 2
        font.pixelSize: Font.sizeToPixels("small")
        text: phoneType
    }

    Text {
        id: phoneItem
        anchors.top: displayNameItem.bottom
        anchors.topMargin: 2
        anchors.left: phoneTypeItem.right
        anchors.leftMargin: 4
        //anchors.right: directionItem.left
        font.pixelSize: Font.sizeToPixels("small")
        text: phone
        transformOrigin: Item.Center
    }
    Text {
        id: dateItem
        anchors.top: phoneTypeItem.bottom
        anchors.topMargin: 2
        anchors.left: photoItem.right
        anchors.leftMargin: 2
        anchors.right: directionItem.left
        font.pixelSize: Font.sizeToPixels("small")
        text: date
    }

    Image {
        id: directionItem
        source: {
            if(direction == "incoming") {
                if(missed) {
                    "../assets/icon_missed_call.png"
                } else {
                    "../assets/icon_incoming_call.png"
                }
            } else {
                "../assets/icon_outgoing_call.png"
            }
        }
        width: 48
        height: 48
        anchors.rightMargin: 1
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
    }

    MouseArea {
        anchors.fill: parent
        onClicked: callItem.clicked(index)
    }

}
