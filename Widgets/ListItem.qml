import QtQuick 1.1

AbstractButton {
    id: listDelegate

    width: 250
    height: (isIcon && !__hasSubLabel ? 30 : 54) + (topSeparator ? 2 : 0) + (bottomSeparator ? 2 : 0)

    property bool isIcon: false // FIXME: not nice
    property alias iconSource: icon.source // FIXME: think of a way to have the selected state done automatically
    property alias text: label.text
    property alias subtext: sublabel.text
    property alias sussubtext: subsublabel.text
    property bool selected: false
    property bool topSeparator: false
    property bool bottomSeparator: true

    property int __padding: 10
    property bool __hasSubLabel: subtext != ""
    property color __textColor: selected ? "#f37505" : Qt.rgba(0.4, 0.4, 0.4, 1.0)

    Rectangle {
        // FIXME: check conformance with design
        id: background

        visible: listDelegate.selected
        anchors.fill: parent
        anchors.topMargin: 1
        color: "white"
        opacity: 0.8
    }

    Image {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 2
        source: "artwork/ListItemSeparator.png"
        visible: listDelegate.topSeparator
    }

    Image {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 2
        source: "artwork/ListItemSeparator.png"
        visible: listDelegate.bottomSeparator
    }

    Item {
        id: iconContainer

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        width: 54

        Image {
            id: icon

            anchors.centerIn: parent
            anchors.verticalCenterOffset: (topSeparator ? 1 : 0) + (bottomSeparator ? 0 : 1)
            width: listDelegate.isIcon ? 16 : 36
            height: width
            sourceSize.width: width
            fillMode: Image.PreserveAspectFit
            opacity: listDelegate.enabled ? 1.0 : 0.5
        }

        BorderImage {
            id: frame

            visible: !listDelegate.isIcon
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

        property bool centered: !listDelegate.__hasSubLabel

        anchors.top: !centered ? parent.top : undefined
        anchors.topMargin: !centered ? listDelegate.__padding - 2 : 0
        anchors.left: iconContainer.right
        anchors.leftMargin: 1
        anchors.right: parent.right
        anchors.rightMargin: listDelegate.__padding
        anchors.verticalCenter: centered ? parent.verticalCenter : undefined
        anchors.verticalCenterOffset: centered ? (topSeparator ? 0 : -1) + (bottomSeparator ? 0 : 1) : 0
        fontSize: "large"
        elide: Text.ElideRight

        color: listDelegate.__textColor
        style: Text.Raised
        styleColor: "white"
        opacity: listDelegate.enabled ? 1.0 : 0.5
    }

    TextCustom {
        id: sublabel

        anchors.left: label.anchors.left
        anchors.leftMargin: label.anchors.leftMargin
        anchors.top: label.bottom
        anchors.right: parent.right
        anchors.rightMargin: listDelegate.__padding
        fontSize: "medium"
        elide: Text.ElideRight

        color: listDelegate.__textColor
        style: Text.Raised
        styleColor: "white"
        opacity: listDelegate.enabled ? 1.0 : 0.5
    }

    TextCustom {
        id: subsublabel

        anchors.baseline: parent.bottom
        anchors.baselineOffset: -listDelegate.__padding + 2
        anchors.right: parent.right
        anchors.rightMargin: listDelegate.__padding
        horizontalAlignment: Text.AlignRight
        fontSize: "small"

        color: listDelegate.__textColor
        style: Text.Raised
        styleColor: "white"
        opacity: listDelegate.enabled ? 1.0 : 0.5
    }
}
