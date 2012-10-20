import QtQuick 2.0
import Ubuntu.Components 0.1

Item {
    id: delegate

    property string fontColor: Qt.rgba(0.4, 0.4, 0.4, 1.0)
    property string fontSize: "medium"
    property bool isLast: false

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

            Item {
                anchors.left: parent.left
                anchors.right: parent.right
                height: 35

                BorderImage {
                    id: background

                    anchors.fill: parent
                    anchors.bottomMargin: delegate.isLast ? -3 : 0
                    source: delegate.isLast ? "../assets/input_field_autofill_bottom.sci" : "../assets/input_field_autofill_middle.sci"
                }

                Item {
                    id: labels

                    anchors.left: parent.left
                    anchors.leftMargin: 12
                    anchors.right: parent.right
                    anchors.rightMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    height: childrenRect.height

                    TextCustom {
                        id: contactName

                        anchors.left: parent.left
                        text: contact.displayLabel
                        fontSize: delegate.fontSize
                        color: fontColor
                        style: Text.Raised
                        styleColor: "white"
                    }

                    TextCustom {
                        anchors.left: contactName.right
                        anchors.leftMargin: 10
                        anchors.right: parent.right
                        horizontalAlignment: Text.AlignRight
                        elide: Text.ElideRight
                        text: modelData.number
                        color: "dark gray"
                        fontSize: delegate.fontSize
                        style: Text.Raised
                        styleColor: "white"
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: delegate.contactClicked(contact, modelData.number)
                }
            } // Rectangle
        } // Repeater
    } // Column
}
