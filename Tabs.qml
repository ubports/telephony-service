import QtQuick 1.1
import "Widgets"

Item {
    id: tabs

    width: 320
    height: 36

    property int currentTab

    Row {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 5

        Button {
            id: phoneTab
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: height
            onClicked: tabs.currentTab = 0
            iconSource: tabs.currentTab == 0 ? "assets/tab_icon_call_active.png" : "assets/tab_icon_call_inactive.png"
        }

        Button {
            id: messagesTab
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: height
            onClicked: tabs.currentTab = 1
            iconSource: tabs.currentTab == 1 ? "assets/tab_icon_messaging_active.png" : "assets/tab_icon_messaging_inactive.png"
        }

        Button {
            id: addressbookTab
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: height
            onClicked: tabs.currentTab = 2
            iconSource: tabs.currentTab == 2 ? "assets/tab_icon_contacts_active.png" : "assets/tab_icon_contacts_inactive.png"
        }
    }

    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: "white"
    }
}
