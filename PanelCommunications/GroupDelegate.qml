import QtQuick 2.0
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem
import "../Widgets" as LocalWidgets
import "../dateUtils.js" as DateUtils

Item {
    id: groupDelegate
    anchors.fill: parent

    LocalWidgets.CustomListItemBase {
        id: infoBox

        anchors.left: parent.left
        anchors.right: parent.right
        height: parent.height
        iconSource: (contactAvatar != "") ? contactAvatar : placeholderIconSource
        placeholderIconSource: "../assets/avatar_contacts_list.png"
        text: contactAlias
        subtext: DateUtils.formatLogDate(timestamp)
        onClicked: groupDelegate.parent.clicked(mouse)
        //selected: callItem.selected
    }

    Row {
        id: indicators

        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: units.gu(0.5)
        anchors.rightMargin: units.gu(1)
        width: childrenRect.width
        height: childrenRect.height
        spacing: units.gu(1)

        // Calls
        Row {
            id: callsRow
            width: childrenRect.width
            height: childrenRect.height
            visible: (typeof(events) != "undefined") && (typeof(events.call) != "undefined")
            spacing: units.gu(0.5)

            Image {
                source: "../assets/contact_icon_phone.png"
            }
            TextCustom {
                text: callsRow.visible ? events.call : ""
                fontSize: "small"
                color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
                style: Text.Raised
                styleColor: "white"
            }
        }

        // Messages
        Row {
            id: messagesRow
            width: childrenRect.width
            height: childrenRect.height
            visible: (typeof(events) != "undefined") && (typeof(events.message) != "undefined")
            spacing: units.gu(0.5)

            Image {
                source: "../assets/contact_icon_message.png"
            }
            TextCustom {
                text: messagesRow.visible ? events.message : ""
                fontSize: "small"
                color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
                style: Text.Raised
                styleColor: "white"
            }
        }
    }
}
