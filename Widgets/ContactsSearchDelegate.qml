import QtQuick 1.1
import "../Widgets"

Item {
    id: delegate
    property string fontColor: "white"
    property string fontSize: "medium"
    property int margins: 5

    signal contactClicked(variant contact, string number)

    height: childrenRect.height

    Column {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: childrenRect.height

        // show one contact entry per phone number in the details
        Repeater {
            model: contact.phoneNumbers

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                height: 50
                color: "gray"

                Rectangle {
                    id: div
                    color: fontColor
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    height: 1
                }


                TextCustom {
                    id: contactName
                    text: contact.displayLabel
                    font.bold: true
                    fontSize: delegate.fontSize
                    color: fontColor
                    anchors.left: parent.left
                    anchors.top: div.bottom
                    anchors.margins: margins
                }

                TextCustom {
                    text: modelData.subTypes.toString()
                    color: fontColor
                    fontSize: delegate.fontSize
                    anchors.right: parent.right
                    anchors.top: div.bottom
                    anchors.margins: margins
                }

                TextCustom {
                    text: modelData.number
                    color: "dark gray"
                    fontSize: delegate.fontSize
                    anchors.left: parent.left
                    anchors.top: contactName.bottom
                    anchors.margins: margins
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: delegate.contactClicked(contact, modelData.number)
                }
            } // Rectangle
        } // Repeater
    } // Column
}
