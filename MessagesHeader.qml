// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

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
        font.pixelSize: 25

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
            font.pixelSize: 15
        }
        Text {
            anchors {
                right: parent.right
                top: number.bottom
            }
            color: "darkGray"
            text: "Mobile"
            font.pixelSize: 15
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
