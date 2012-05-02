// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Item {

    width: 400
    height: 600

    Component {
        id: sectionDelegate

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: 40

            Text {
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 5
                text: section
            }
        }
    }

    Component {
        id: contactDetailsDelegate

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: valueText.height > 30 ? valueText.height : 30

            Rectangle {
                id: contentBox
                border.color: "black"
                color: "white"

                anchors.left: parent.left
                anchors.right: actionBox.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom

                Text {
                    id: valueText
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: 5
                    text: value
                }

                Text {
                    id: typeText
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.rightMargin: 5
                    text: type
                }

            }

            Rectangle {
                id: actionBox
                border.color: "black"
                color: "white"
                // TODO: add the icons
                width: 30
                height: parent.height
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.right: parent.right
            }
        }
    }

    ListView {
        id: contactDetailsView
        anchors.fill: parent
        anchors.margins: 10
        section.delegate: sectionDelegate
        section.property: "section"

        delegate: contactDetailsDelegate

        model: ContactDetailsModel { }
    }
}
