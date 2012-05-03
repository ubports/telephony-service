// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import "fontUtils.js" as Font

Rectangle {
    id: contactList
    width: 320
    height: 600

    Component {
        id: sectionHeading
        Item {
            width: contactList.width
            height: childrenRect.height
            Text {
                text: section
                font.pixelSize: Font.sizeToPixels("medium")
                font.bold: true
            }
            Rectangle {
                height: 1
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                color: "black"
            }
        }
    }

    ListView {
        anchors.fill: parent
        model: contacts
        delegate: Rectangle {
            height: 64
            width: parent.width
            Rectangle {
                height: 1
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                color: "black"
            }
            Image {
                id: photoItem
                source: "dummydata/" + photo
                width: 56
                height: width
                anchors.left: parent.left
                anchors.leftMargin: 1
                anchors.verticalCenter: parent.verticalCenter
            }
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
        }
        section.property: "displayName"
        section.criteria: ViewSection.FirstCharacter
        section.delegate: sectionHeading
    }
}
