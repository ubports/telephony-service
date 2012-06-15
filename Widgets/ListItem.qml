import QtQuick 1.1

AbstractButton {
    id: listItem

    width: 250
    height: (isIcon && !__hasSubLabel ? 30 : 54) + topSeparatorLine.height + bottomSeparatorLine.height

    property bool isIcon: false // FIXME: not nice
    property alias iconSource: icon.source // FIXME: think of a way to have the selected state done automatically
    property alias text: label.text
    property alias subtext: sublabel.text
    property bool selected: false
    property bool topSeparator: false
    property bool bottomSeparator: true

    property int __padding: 10
    property bool __hasSubLabel: subtext != ""
    property color __textColor: selected ? "#f37505" : Qt.rgba(0.4, 0.4, 0.4, 1.0)

    Rectangle {
        id: background

        visible: listItem.selected
        anchors.fill: body
        color: "white"
        opacity: 0.7
    }

    Image {
        id: topSeparatorLine

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: visible ? 2 : 0
        source: "artwork/ListItemSeparator.png"
        visible: listItem.topSeparator
    }

    Image {
        id: bottomSeparatorLine

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: visible ? 2 : 0
        source: "artwork/ListItemSeparator.png"
        visible: listItem.bottomSeparator
    }

    Image {
        visible: listItem.selected
        anchors.top: body.top
        anchors.topMargin: 2
        anchors.right: body.right
        anchors.rightMargin: 2
        source: "../assets/list_item_selected_triangle.png"
    }

    Item {
        id: body

        anchors.top: topSeparatorLine.bottom
        anchors.bottom: bottomSeparatorLine.top
        anchors.left: parent.left
        anchors.right: parent.right

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
                width: listItem.isIcon ? 16 : 36
                height: width
                sourceSize.width: width
                fillMode: Image.PreserveAspectFit
                opacity: listItem.enabled ? 1.0 : 0.5
            }

            BorderImage {
                id: frame

                visible: !listItem.isIcon
                source: "artwork/ListItemFrame.png"
                anchors.fill: icon
                anchors.bottomMargin: -1
                border.left: 3
                border.right: 3
                border.top: 3
                border.bottom: 3
                horizontalTileMode: BorderImage.Stretch
                verticalTileMode: BorderImage.Stretch
            }
        }

        TextCustom {
            id: label

            property bool centered: !listItem.__hasSubLabel

            anchors.top: !centered ? parent.top : undefined
            anchors.topMargin: !centered ? listItem.__padding - 3 : 0
            anchors.left: iconContainer.right
            anchors.leftMargin: 1
            anchors.right: parent.right
            anchors.rightMargin: listItem.__padding
            anchors.verticalCenter: centered ? parent.verticalCenter : undefined
            fontSize: "large"
            elide: Text.ElideRight

            color: listItem.__textColor
            style: Text.Raised
            styleColor: "white"
            opacity: listItem.enabled ? 1.0 : 0.5
        }

        TextCustom {
            id: sublabel

            anchors.left: label.anchors.left
            anchors.leftMargin: label.anchors.leftMargin
            anchors.top: label.bottom
            anchors.topMargin: 1
            anchors.right: parent.right
            anchors.rightMargin: listItem.__padding
            fontSize: "medium"
            elide: Text.ElideRight

            color: listItem.__textColor
            style: Text.Raised
            styleColor: "white"
            opacity: listItem.enabled ? 1.0 : 0.5
        }
    }
}
