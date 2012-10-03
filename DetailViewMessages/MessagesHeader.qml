import QtQuick 1.1
import Ubuntu.Components 0.1

BaseMessageHeader {
    id: header

    property variant contact
    property string number

    TextCustom {
        id: name

        anchors.left: parent.left
        anchors.leftMargin: 13
        anchors.right: icon.left
        anchors.rightMargin: 10
        anchors.verticalCenter: parent.verticalCenter
        fontSize: "large"
        elide: Text.ElideRight
        color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
        style: Text.Raised
        styleColor: "white"
        text: contact ? contact.displayLabel : "Unknown Contact"
    }

    Image {
        id: icon

        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -1
        width: 28
        height: width
        sourceSize.width: width
        fillMode: Image.PreserveAspectFit
        source: contact ? contact.avatar : "../assets/avatar_messaging.png"
        onStatusChanged: if (status == Image.Error) source = "../assets/avatar_messaging.png"
        asynchronous: true
    }

    BorderImage {
        id: iconFrame

        source: "../Widgets/artwork/ListItemFrame.png"
        anchors.fill: icon
        anchors.bottomMargin: -1
        border.left: 3
        border.right: 3
        border.top: 3
        border.bottom: 3
        horizontalTileMode: BorderImage.Stretch
        verticalTileMode: BorderImage.Stretch
    }

    AbstractButton {
        anchors.fill: icon
        onClicked: telephony.showContactDetails(contact)
    }

    Item {
        anchors.right: icon.left
        anchors.rightMargin: 5
        anchors.left: name.right
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -1
        height: childrenRect.height

        TextCustom {
            id: label

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            horizontalAlignment: Text.AlignRight
            fontSize: "small"
            elide: Text.ElideRight
            color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
            style: Text.Raised
            styleColor: "white"
            text: header.number
        }

        TextCustom {
            id: sublabel

            anchors.top: label.bottom
            anchors.topMargin: 1
            anchors.left: parent.left
            anchors.right: parent.right
            horizontalAlignment: Text.AlignRight
            fontSize: "x-small"
            elide: Text.ElideRight
            color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
            style: Text.Raised
            styleColor: "white"
            text: contact ? contact.phoneType : ""
        }
    }
}
