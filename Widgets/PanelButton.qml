import QtQuick 1.1
import Ubuntu.Components 0.1

AbstractButton {
    id: button

    width: 250
    height: 30

    property alias iconSource: icon.source // FIXME: think of a way to have the selected state done automatically
    property alias text: label.text
    property bool selected: false

    property color __textColor: (selected) ? "#f37505" : Qt.rgba(0.4, 0.4, 0.4, 1.0)

    Image {
        visible: button.selected
        anchors.top: body.top
        anchors.topMargin: 2
        anchors.right: body.right
        anchors.rightMargin: 2
        source: "../assets/list_item_selected_triangle.png"
    }

    Item {
        id: body

        anchors.fill: parent

        Item {
            id: iconContainer

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            width: 54

            Image {
                id: icon

                anchors.centerIn: parent
                anchors.verticalCenterOffset: 1
                width: 16
                sourceSize.width: width
                fillMode: Image.PreserveAspectFit
                opacity: button.enabled ? 1.0 : 0.5
                asynchronous: true
            }
        }

        TextCustom {
            id: label

            property bool centered: true

            anchors.left: iconContainer.right
            anchors.leftMargin: 1
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            fontSize: "large"
            elide: Text.ElideRight

            color: button.__textColor
            style: Text.Raised
            styleColor: "white"
            opacity: button.enabled ? 1.0 : 0.5
        }
    }
}
