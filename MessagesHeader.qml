import QtQuick 1.1
import "fontUtils.js" as Font

Item {
    width: 575
    height: 100    

    Text {
        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
            leftMargin: 20
        }
        color: "black"
        text: "Anna Olsson"
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
            text: "+44 794 323 439"
            font.pixelSize: Font.sizeToPixels("medium")
        }
        Text {
            anchors {
                right: parent.right
                top: number.bottom
            }
            color: "darkGray"
            text: "Mobile"
            font.pixelSize: Font.sizeToPixels("medium")
        }
    }


    Rectangle {
        id: image
        color: "gray"
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
