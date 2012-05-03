import QtQuick 1.1
import "fontUtils.js" as Font

Item {
    width: 575
    height: 100    
    property variant contact: null

    Component.onCompleted: {
        contact = contacts.get(3)
    }

    Text {
        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
            leftMargin: 20
        }
        color: "black"
        text: contact.displayName
        font.pixelSize: Font.sizeToPixels("x-large")

    }

    Item {
        id: details
        height: childrenRect.height
        anchors {
            right: image.left
            rightMargin: 20
            verticalCenter: image.verticalCenter
        }

        Text {
            id: number
            anchors {
                right: parent.right
                top: parent.top
            }
            color: "darkGray"
            text: contact.phone
            font.pixelSize: Font.sizeToPixels("medium")
        }
        Text {
            anchors {
                right: parent.right
                top: number.bottom
            }
            color: "darkGray"
            text: contact.phoneType
            font.pixelSize: Font.sizeToPixels("medium")
        }
    }


    Image {
        id: image
        //color: "gray"
        source: "dummydata/" + contact.photo
        width: height
        anchors {
            right: parent.right
            rightMargin: 20
            top: parent.top
            bottom: parent.bottom
            topMargin: 10
            bottomMargin: 10
        }
    }

    Rectangle {
        id: line
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        height: 1
        color: "gray"
    }


}
