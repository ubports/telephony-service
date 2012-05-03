import QtQuick 1.1

Rectangle {
    id: contactsPanel
    anchors.fill: parent
    signal contactClicked(variant contact)
    onContactClicked: telephony.showContactDetails(contact)

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
            onItemSelected: telephony.startCall(item)
        }
        z: 1
    }

    Flickable {
        contentHeight: favouriteContactList.height + contactList.height
        flickableDirection: Flickable.VerticalFlick
        anchors.top: searchArea.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        clip: true

        FavouriteContactList {
            id: favouriteContactList
            clip: true
            anchors.top: parent.top
            onContactClicked: contactsPanel.contactClicked(contact)
        }

        ContactList {
            id: contactList
            anchors.top: favouriteContactList.bottom
            clip: true
            onContactClicked: contactsPanel.contactClicked(contact)
        }
    }
}
