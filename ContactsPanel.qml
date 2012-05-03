import QtQuick 1.1

Rectangle {
    anchors.fill: parent
    Rectangle {
        id: searchArea
        height: 50
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        ContactsSearchCombo {
            id: contactsSearchBox
            anchors.verticalCenter: parent.verticalCenter
            height: 30
            width: 250
        }
        z: 1
    }

    Flickable {
        contentHeight: fcl.height + cl.height
        flickableDirection: Flickable.VerticalFlick
        anchors.top: searchArea.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        clip: true
        FavouriteContactList {
            id: fcl
            clip: true
            anchors.top: parent.top
        }

        ContactList {
            id: cl
            anchors.top: fcl.bottom
            clip: true
        }
    }
}
