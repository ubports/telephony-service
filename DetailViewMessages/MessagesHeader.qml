import QtQuick 1.1
import "../Widgets"

Item {
    id: header

    property variant contact

    TextCustom {
        id: name
        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
            leftMargin: 20
        }
        color: "black"
        text: contact ? contact.displayName : ""
        fontSize: "x-large"
    }

    Item {
        id: details
        height: childrenRect.height
        anchors {
            right: image.left
            rightMargin: 20
            verticalCenter: image.verticalCenter
        }

        TextCustom {
            id: number
            anchors {
                right: parent.right
                top: parent.top
            }
            color: "darkGray"
            text: contact ? contact.phone : ""
            fontSize: "medium"
        }
        TextCustom {
            anchors {
                right: parent.right
                top: number.bottom
            }
            color: "darkGray"
            text: contact ? contact.phoneType : ""
            fontSize: "medium"
        }
    }

    Image {
        id: image
        source: contact ? "../dummydata/" + contact.photo : ""
        width: height
        anchors {
            right: parent.right
            rightMargin: 20
            top: parent.top
            bottom: parent.bottom
            topMargin: 10
            bottomMargin: 10
        }

        MouseArea {
            anchors.fill: parent
            onClicked: telephony.showContactDetails(contact)
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
