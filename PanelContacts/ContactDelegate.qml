import QtQuick 2.0
import Ubuntu.Components.ListItems 0.1 as ListItem
import Ubuntu.Components 0.1

ListItem.Standard {
    width: (parent) ? parent.width : undefined
    __height: units.gu(8)
    showDivider: true
    property variant model

    UbuntuShape {
        id: avatar
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: units.gu(2)
        height: units.gu(6)
        width: units.gu(6)
        image: Image {
            anchors.fill: parent
            source: model.decoration != "" ? Qt.resolvedUrl(model.decoration) : "../assets/avatar-default.png"
            asynchronous: true
        }
    }

    Label {
        text: model.display
        anchors.top: avatar.top
        anchors.left: avatar.right
        anchors.leftMargin: units.gu(1)
        fontSize: "medium"
    }
}
