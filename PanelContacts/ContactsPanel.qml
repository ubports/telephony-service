import QtQuick 1.1
import ".."
import "../Widgets"
import "../fontUtils.js" as Font

Rectangle {
    id: contactsPanel

    property alias searchQuery : contactsSearchBox.searchQuery

    // FIXME: move anchor and signal handler to instance
    anchors.fill: parent
    signal contactClicked(variant contact)
    onContactClicked: telephony.showContactDetails(contact)

    SearchEntry {
        id: contactsSearchBox
        height: 30
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 5

        leftIconSource: text ? "../assets/cross.png" : "../assets/search_icon.png"
        onLeftIconClicked: text = ""
    }

    ListView {
        id: contactsList
        anchors.top: contactsSearchBox.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 5
        clip: true

        ListModel {
            id: contactsModel
            Component.onCompleted: {
                var contact
                // favourites
                for(var i = 0; i < contacts.count; i++) {
                    contact = contacts.get(i)
                    if (contact.favourite) {
                        contact.sectionName = "Favourites"
                        append(contact)
                    }
                }
                // all contacts
                for(var i = 0; i < contacts.count; i++) {
                    contact = contacts.get(i)
                    contact.sectionName = contact.displayName.substring(0, 1)
                    append(contact)
                }
            }
        }

        FilterModel {
            id: filterModel
            filter: contactsPanel.searchQuery
            proxyModel: contactsModel
            fields: ["displayName", "phone"]
        }
        model: filterModel.model

        delegate: ContactDelegate {
            onClicked: contactsPanel.contactClicked(contactsList.model.get(index))
        }

        section.property: "sectionName"
        // FIXME: use TextCustom instead of Text
        section.delegate: Text {
            width: parent.width
            height: paintedHeight
            text: section
            font.pixelSize: Font.sizeToPixels("medium")
            font.bold: true
            Rectangle {
                height: 1
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                color: "black"
            }
        }
    }
}
