import QtQuick 2.0
import "../Widgets" as LocalWidgets
import "../dateUtils.js" as DateUtils
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem

Item {
    id: callItem

    state: item.incoming ? "incoming" : "outgoing"

    states: [
        State {
            name: "incoming"
            PropertyChanges {
                target: separator
                anchors.right: actionBox.left
                anchors.left: undefined
            }
            PropertyChanges {
                target: actionBox 
                anchors.right: parent.right
                anchors.left: undefined
            }
            PropertyChanges {
                target: timeLabel
                anchors.right: undefined
                anchors.left: parent.left
                horizontalAlignment: Text.AlignLeft
            }
        },
        State {
            name: "outgoing"
            PropertyChanges {
                target: separator
                anchors.left: actionBox.right
                anchors.right: undefined
            }
            PropertyChanges {
                target: actionBox
                anchors.right: undefined
                anchors.left: parent.left
            }
            PropertyChanges {
                target: timeLabel
                anchors.right: parent.right
                anchors.left: undefined
                horizontalAlignment: Text.AlignRight
            }
        }
    ]

    LocalWidgets.CustomListItemBase {
        id: infoBox

        incoming: item.incoming
        anchors.left: parent.left
        anchors.right: parent.right
        height: units.gu(7)
        iconSource: (contactAvatar != "") ? contactAvatar : placeholderIconSource
        placeholderIconSource: "../assets/avatar_contacts_list.png"
        text: contactAlias
        subtext: item.phoneNumber

        onClicked: callItem.parent.clicked(mouse)
    }

    TextCustom {
        id: timeLabel

        parent: infoBox.content
        anchors.rightMargin: units.gu(1)
        anchors.bottom: parent.bottom
        anchors.bottomMargin: units.gu(0.5)
        fontSize: "small"
        color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
        style: Text.Raised
        styleColor: "white"
        text: DateUtils.formatLogDate(timestamp)
    }

    Row {
        id: indicators

        parent: infoBox.content
        anchors.left: timeLabel.right
        anchors.verticalCenter: timeLabel.verticalCenter

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

    Rectangle {
        id: separator

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: actionBox.left
        width: units.dp(1)
        color: "black"
        opacity: 0.1
        visible: item.phoneNumber != "-"
    }

    LocalWidgets.CustomListItemBase {
        id: actionBox

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.rightMargin: units.dp(2)
        anchors.topMargin: units.dp(1)
        width: height

        iconSource: item.phoneNumber != "-" ? "../assets/tab_icon_call_inactive.png" : ""
        isIcon: true
        onClicked: {
            if (item.phoneNumber != "-") {
                telephony.callNumber(item.phoneNumber)
            }
        }
    }
}
