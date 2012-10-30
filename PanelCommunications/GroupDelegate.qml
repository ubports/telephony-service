import QtQuick 2.0
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem
import "../Widgets" as LocalWidgets
import "../dateUtils.js" as DateUtils

ListItem.Base {
    anchors.left: parent.left
    anchors.right: parent.right
    showDivider: true
    __height: 58

    LocalWidgets.CustomListItemBase {
        id: infoBox

        anchors.left: parent.left
        anchors.right: parent.right
        height: parent.height
        iconSource: (contactAvatar != "") ? contactAvatar : placeholderIconSource
        placeholderIconSource: "../assets/avatar_contacts_list.png"
        text: contactAlias
        subtext: DateUtils.formatLogDate(timestamp)
        //onClicked: callItem.clicked(contactId)
        //selected: callItem.selected
    }

    Row {
        id: indicators

        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 2
        anchors.rightMargin: 10
        width: childrenRect.width
        height: childrenRect.height
        spacing: 10

        // Calls
        Row {
            width: childrenRect.width
            height: childrenRect.height
            visible: typeof(events.call) != "undefined"

            Image {
                source: "../assets/contact_icon_phone.png"
            }
            TextCustom {
                text: events.call
                fontSize: "small"
                color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
                style: Text.Raised
                styleColor: "white"
            }
        }

        // Messages
        Row {
            width: childrenRect.width
            height: childrenRect.height
            visible: typeof(events.message) != "undefined"

            Image {
                source: "../assets/contact_icon_message.png"
            }
            TextCustom {
                text: events.message
                fontSize: "small"
                color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
                style: Text.Raised
                styleColor: "white"
            }
        }
    }
}
