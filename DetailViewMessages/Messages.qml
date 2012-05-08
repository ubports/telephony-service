import QtQuick 1.1
import "../Widgets"

Item {
    id: messages

    width: 600
    height: 100

    property ListModel model: messagesModel

    // private data
    property int newMessageMinutes: 35 // helper for new messages text

    function addMessage(newMessage) {
        messages.model.append({"section": "Friday, May 04, 2012",
                               "message": newMessage,
                               "timeStamp": "3:" + newMessageMinutes + " PM",
                               "outgoing": true})
        newMessageMinutes++;
        //FIX: try to use list view methods better
        messagesList.positionViewAtIndex(model.count - 1, ListView.End)
    }

    Component {
        id: sectionDelegate
        Item {
            height: sectionText.height + line.anchors.bottomMargin

            TextCustom {
                id: sectionText
                text: section
                anchors.bottom: line.top
                fontSize: "medium"
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

            TextCustom {
                id: messageText
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                    margins: 5
                }

                text: message
                wrapMode: Text.WordWrap
                fontSize: "large"
            }

            TextCustom {
                id: timeText
                text: timeStamp
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                anchors.rightMargin: 5
                color: "gray"
                fontSize: "small"
            }
        }
    }

    ListView {
        id: messagesList

        anchors.fill: parent
        contentWidth: parent.width
        contentHeight: messages.height
        clip: true
        spacing: 8
        orientation: ListView.Vertical
        model: messages.model
        section.delegate: sectionDelegate
        section.property: "section"
        delegate: messageDelegate
    }

}
