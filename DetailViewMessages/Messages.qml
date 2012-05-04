import QtQuick 1.1
import "../fontUtils.js" as Font

Item {
    id: messages
    width: 600
    height: 100
    property ListModel model: messagesModel

    function addMessage(newMessage) {
        messages.model.append({"section": "Friday, May 01, 2012",
                               "message": newMessage,
                               "timeStamp": "3:35 PM",
                               "outgoing": true})
        flickable.contentY = flickable.contentHeight
    }

    Component {
        id: sectionDelegate
        Item {
            height: sectionText.height + line.anchors.bottomMargin

            Text {
                id: sectionText
                text: section
                anchors.bottom: line.top
                font.pixelSize: Font.sizeToPixels("medium")
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
                x: 10
                y: 10
                font.pixelSize: Font.sizeToPixels("large")
            }

            Text {
                id: timeText
                text: timeStamp
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                anchors.rightMargin: 5
                color: "gray"
                font.pixelSize: Font.sizeToPixels("small")
            }
        }
    }

    ListView {
        id: flickable
        anchors {
            fill: parent
            //topMargin: 10
        }
        contentWidth: parent.width;
        contentHeight: messages.height
        clip: true
        spacing: 8
        orientation: ListView.Vertical
        model: messages.model
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

    PropertyAnimation {
        id: contentAnimation
        target: flickable
        properties: "contentY"
        duration: 300
        easing.type: Easing.InOutCubic
    }
}
