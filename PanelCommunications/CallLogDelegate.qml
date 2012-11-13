import QtQuick 2.0
import "../Widgets" as LocalWidgets
import "../dateUtils.js" as DateUtils
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem

Item {
    id: callItem

    LocalWidgets.CustomListItemBase {
        id: infoBox

        anchors.left: parent.left
        anchors.right: parent.right
        height: units.gu(7)
        iconSource: (contactAvatar != "") ? contactAvatar : placeholderIconSource
        placeholderIconSource: "../assets/avatar_contacts_list.png"
        text: contactAlias
        subtext: item.phoneNumber
        selected: callItem.parent ? callItem.parent.selected : false

        onClicked: callItem.parent.clicked(mouse)
    }

    TextCustom {
        id: timeLabel

        anchors.right: parent.right
        anchors.rightMargin: units.gu(1)
        anchors.bottom: infoBox.bottom
        anchors.bottomMargin: units.gu(0.5)
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
                if(item.incoming) {
                    if(item.missed) {
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
}
