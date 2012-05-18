import QtQuick 1.1
import QtMobility.contacts 1.1
import ".."
import "../Widgets"
import "../fontUtils.js" as Font

Rectangle {
    id: contactsPanel

    property alias searchQuery : contactsSearchBox.searchQuery

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

    ContactModel {
        id: contactsModel
        manager: "folks"

        filter: UnionFilter {
            DetailFilter {
                detail: ContactDetail.DisplayLabel
                field: DisplayLabel.label
                value: contactsSearchBox.text
                matchFlags: DetailFilter.MatchContains
            }

            DetailFilter {
                detail: ContactDetail.NickName
                field: Nickname.nickname
                value: contactsSearchBox.text
                matchFlags: DetailFilter.MatchContains
            }

            DetailFilter {
                detail: ContactDetail.Presence
                field: Presence.nickname
                value: contactsSearchBox.text
                matchFlags: DetailFilter.MatchContains
            }
        }
    }

    ListView {
        id: contactsList
        anchors.top: contactsSearchBox.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 5
        clip: true
        model: contactsModel
        delegate: ContactDelegate {
            onClicked: contactsPanel.contactClicked(contact)
        }

        // FIXME: check how to use sections with the QtMobility models
        section.property: "sectionName"
        section.delegate: TextCustom {
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
