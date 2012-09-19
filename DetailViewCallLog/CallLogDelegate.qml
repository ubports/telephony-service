import QtQuick 1.1
import "../Widgets"
import "../dateUtils.js" as DateUtils
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem

ListItem.Base {
    id: callItem

    signal clicked(string contactId)
    signal actionClicked(string contactId, string phoneNumber)

    height: infoBox.height

    CustomListItemBase {
        id: infoBox

        anchors.left: parent.left
        anchors.right: separator.left
        height: 60
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
        anchors.rightMargin: 8
        anchors.bottom: infoBox.bottom
        anchors.bottomMargin: 5
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
        anchors.bottomMargin: 3

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
        width: 1
        color: "black"
        opacity: 0.1
        visible: phoneNumber != "-"
    }

    CustomListItemBase {
        id: actionBox

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
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
