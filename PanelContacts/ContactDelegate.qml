import QtQuick 1.1
import "../fontUtils.js" as Font

Item {
    height: 64
    width: parent.width
    signal clicked
    Rectangle {
        height: 1
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: "black"
    }
    Image {
        id: photoItem
        // FIXME: move the dummydata/ prefix to the model
        source: "../dummydata/" + photo
        width: 56
        height: width
        anchors.left: parent.left
        anchors.leftMargin: 1
        anchors.verticalCenter: parent.verticalCenter
    }
    // FIXME: use TextCustom instead of Text
    Text {
        id: displayNameItem
        anchors.top: parent.top
        anchors.topMargin: 1
        anchors.left: photoItem.right
        anchors.leftMargin: 2
        font.pixelSize: Font.sizeToPixels("medium")
        text: displayName
    }
    Text {
        id: emailItem
        anchors.top: displayNameItem.bottom
        anchors.topMargin: 2
        anchors.left: photoItem.right
        anchors.leftMargin: 2
        font.pixelSize: Font.sizeToPixels("small")
        text: email
    }
    Text {
        id: phoneItem
        anchors.top: emailItem.bottom
        anchors.topMargin: 2
        anchors.left: photoItem.right
        anchors.leftMargin: 2
        font.pixelSize: Font.sizeToPixels("small")
        text: phone
    }
    MouseArea {
        anchors.fill: parent
        onClicked: parent.clicked()
    }
}
