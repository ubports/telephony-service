// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Item {
    id: messages
    width: 600
    height: 100

    Component {
        id: sectionDelegate
        Item {
            height: sectionText.height + line.anchors.bottomMargin

            Text {
                id: sectionText
                text: section
                anchors.bottom: line.top
            }

            Rectangle {
                id: line
                color: "black"
                width: messages.width
                height: 1
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 5
            }
        }
    }

    Component {
        id: messageDelegate
        Rectangle {
            id: bg
            color: outgoing ? "white" : "darkGray"
            height: messageText.paintedHeight + 40

            radius: 0
            border.color: "black"
            border.width: 1
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: outgoing ? 10 : 1/3 * messages.width
                rightMargin: outgoing ? 1/3 * messages.width : 10
            }

            Text {
                id: messageText
                width: parent.width
                text: message
                wrapMode: Text.WordWrap
                font.pixelSize: 20
                x: 10
                y: 10
            }

            Text {
                id: timeText
                text: timeStamp
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                anchors.rightMargin: 5
                color: "gray"
            }
        }
    }

    ListView {
        id: flickable
        anchors.fill: parent        
        contentWidth: parent.width;
        contentHeight: messages.height
        clip: true
        spacing: 8
        orientation: ListView.Vertical
        model: MessagesModel{}
        section.delegate: sectionDelegate
        section.property: "section"
        delegate: messageDelegate
    }

    Rectangle {
        id: line
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        height: 1
        color: "gray"
    }
}
