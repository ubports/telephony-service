// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import "fontUtils.js" as Font

Rectangle {
    id: contactList
    width: 320
    // FIXME - calculate headers + body size automatically
    height: 600

    Component {
        id: sectionHeading
        Item {
            width: contactList.width
            height: sectionHeaderText.height
            Text {
                id: sectionHeaderText
                text: section
                font.pixelSize: Font.sizeToPixels("medium")
                font.bold: true
            }
            Rectangle {
                height: 1
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                color: "black"
            }
        }
    }

    ListView {
        anchors.fill: parent
        interactive: false
        model: contacts
        delegate: ContactDelegate {}
        section.property: "displayName"
        section.criteria: ViewSection.FirstCharacter
        section.delegate: sectionHeading
    }
}
