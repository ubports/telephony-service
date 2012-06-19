import QtQuick 1.1
import TelephonyApp 0.1
import "../Widgets"

Item {
    id: messages
    property variant contact
    property string number

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
                leftMargin: incoming ? 1/3 * messages.width : 10
                rightMargin: incoming ? 10 : 1/3 * messages.width
            }

            Rectangle {
                anchors.fill: parent
                anchors.margins: 1
                color: incoming ? "darkGray" : "white"
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
                text: Qt.formatDateTime(timestamp, Qt.DefaultLocaleShortDate)
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                anchors.rightMargin: 5
                color: "gray"
                fontSize: "small"
            }
        }
    }

    MessagesProxyModel {
        id: messagesProxyModel
        messagesModel: messageLogModel
        ascending: true;
    }

    ListView {
        id: messagesList

        anchors.fill: parent
        contentWidth: parent.width
        contentHeight: messages.height
        clip: true
        spacing: 8
        orientation: ListView.Vertical
        ListModel { id: messagesModel }
        // FIXME: references to runtime and fake model need to be removed before final release
        model: typeof(runtime) != "undefined" ? fakeMessagesModel : messagesProxyModel
        section.delegate: sectionDelegate
        section.property: "date"
        delegate: messageDelegate
        highlightFollowsCurrentItem: true
        currentIndex: (count > 0) ? count-1 : 0
    }

}
