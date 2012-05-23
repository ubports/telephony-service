import QtQuick 1.1
import "../Widgets"

Item {
    id: messages

    property ListModel model: messagesModel

    // private data
    property int newMessageMinutes: 35 // helper for new messages text

    function addMessage(newMessage) {
        var currentDate = new Date()
        messagesList.model.append({"section": Qt.formatDate(currentDate, Qt.DefaultLocaleLongDate),
                               "message": newMessage,
                               "timeStamp": Qt.formatTime(currentDate, Qt.DefaultLocaleLongDate),
                               "outgoing": true})
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
        Item {
            id: bg

            height: messageText.paintedHeight + 40
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: outgoing ? 10 : 1/3 * messages.width
                rightMargin: outgoing ? 1/3 * messages.width : 10
            }

            Rectangle {
                anchors.fill: parent
                anchors.margins: 1
                color: outgoing ? "white" : "darkGray"
                border.color: "black"
                border.width: 1
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
        model: ListModel { }
        section.delegate: sectionDelegate
        section.property: "section"
        delegate: messageDelegate
        highlightFollowsCurrentItem: true
        currentIndex: (count > 0) ? count-1 : 0
    }

}
