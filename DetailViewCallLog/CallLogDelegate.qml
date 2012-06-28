import QtQuick 1.1
import "../Widgets"

Item {
    id: callItem

    signal clicked(string contactId)
    signal actionClicked(string contactId, string phoneNumber)

    height: infoBox.height

    ListItem {
        id: infoBox

        anchors.left: parent.left
        anchors.right: separator.left
        iconSource: avatar
        placeholderIconSource: "../assets/avatar_contacts_list.png"
        text: contactAlias
        subtext: phoneNumber
        onClicked: callItem.clicked(contactId)
    }

    TextCustom {
        id: timeLabel

        anchors.right: separator.left
        anchors.rightMargin: 8
        anchors.bottom: infoBox.bottom
        anchors.bottomMargin: 5
        fontSize: "medium"
        color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
        style: Text.Raised
        styleColor: "white"
        // FIXME: show time if today otherwise date without year
        text: Qt.formatDateTime(timestamp, Qt.DefaultLocaleShortDate)
    }

    Row {
        id: indicators

        anchors.left: timeLabel.left
        anchors.right: timeLabel.right
        anchors.bottom: timeLabel.top
        anchors.bottomMargin: 2

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
    }

    ListItem {
        id: actionBox

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: height

        iconSource: "../assets/tab_icon_call_inactive.png"
        isIcon: true
        topSeparator: infoBox.topSeparator
        bottomSeparator: infoBox.bottomSeparator
        onClicked: callItem.actionClicked(contactId, phoneNumber)
    }
}
