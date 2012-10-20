import QtQuick 2.0
import "../Widgets" as LocalWidgets
import "../dateUtils.js" as DateUtils
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem

ListItem.Base {
    id: callItem

    signal contactClicked(string contactId)
    signal actionClicked(string contactId, string phoneNumber)

    __height: infoBox.height
    showDivider: true

    LocalWidgets.CustomListItemBase {
        id: infoBox

        anchors.left: parent.left
        anchors.right: separator.left
        height: units.dp(58)
        iconSource: (avatar != "") ? avatar : placeholderIconSource
        placeholderIconSource: "../assets/avatar_contacts_list.png"
        text: contactAlias
        subtext: phoneNumber
        onClicked: callItem.clicked(contactId)
        selected: callItem.selected
    }

    TextCustom {
        id: timeLabel

        anchors.right: separator.left
        anchors.rightMargin: units.dp(9)
        anchors.bottom: infoBox.bottom
        anchors.bottomMargin: units.dp(3)
        fontSize: "small"
        color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
        style: Text.Raised
        styleColor: "white"
        text: DateUtils.formatLogDate(timestamp)
    }

    Row {
        id: indicators

        anchors.left: timeLabel.left
        anchors.right: timeLabel.right
        anchors.bottom: timeLabel.top
        anchors.bottomMargin: units.dp(2)

        Image {
            source: {
                if(incoming) {
                    if(missed) {
                        "../assets/missed_call.png"
                    } else {
                        "../assets/received_call.png"
                    }
                } else {
                    "../assets/outgoing_call.png"
                }
            }
        }
    }

    Rectangle {
        id: separator

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: actionBox.left
        width: units.dp(1)
        color: "black"
        opacity: 0.1
        visible: phoneNumber != "-"
    }

    LocalWidgets.CustomListItemBase {
        id: actionBox

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.rightMargin: units.dp(2)
        anchors.topMargin: units.dp(1)
        width: height
        selected: callItem.selected

        iconSource: phoneNumber != "-" ? "../assets/tab_icon_call_inactive.png" : ""
        isIcon: true
        onClicked: {
            if (phoneNumber != "-") {
                callItem.actionClicked(contactId, phoneNumber)
            }
        }
    }
}
