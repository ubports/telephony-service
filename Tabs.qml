// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Rectangle {
    id: tabs

    width: 320
    height: width / 3

    color: "black"

    property int currentTab
    property real imageSize: height / 2

    Image {
        id: arrow
        source: "assets/tab_icon_arrow.png"
        width: 24
        height: 12
        anchors.bottom: parent.bottom

        anchors.horizontalCenter: {
            if (tabs.currentTab == 0) return phoneTab.horizontalCenter
            else if (tabs.currentTab == 1) return messagesTab.horizontalCenter
            else if (tabs.currentTab == 2) return addressbookTab.horizontalCenter
        }
    }

    MouseArea {
        id: phoneTab
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: height
        onClicked: tabs.currentTab = 0
        Image {
            source: "assets/tab_icon_phone.png"
            width: tabs.imageSize
            height: tabs.imageSize
            anchors.centerIn: parent
        }
    }

    MouseArea {
        id: messagesTab
        anchors.left: phoneTab.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: height
        onClicked: tabs.currentTab = 1
        Image {
            source: "assets/tab_icon_messages.png"
            width: tabs.imageSize
            height: tabs.imageSize
            anchors.centerIn: parent
        }
    }

    MouseArea {
        id: addressbookTab
        anchors.left: messagesTab.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: height
        onClicked: tabs.currentTab = 2
        Image {
            source: "assets/tab_icon_address_book.png"
            width: tabs.imageSize
            height: tabs.imageSize
            anchors.centerIn: parent
        }
    }
}
