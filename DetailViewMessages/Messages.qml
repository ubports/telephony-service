import QtQuick 1.1
import TelephonyApp 0.1
import "../Widgets"

Item {
    id: messages
    property variant contact
    property string number

    clip: true

    Component {
        id: sectionDelegate

        Item {
            height: childrenRect.height + 13

            TextCustom {
                anchors.left: parent.left
                anchors.leftMargin: 16
                text: section
                fontSize: "medium"
                elide: Text.ElideRight
                color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
                style: Text.Raised
                styleColor: "white"
            }
        }
    }

    Component {
        id: messageDelegate

        Item {
            height: textBubble.visible ? textBubble.height : imageBubble.height
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: incoming ? 40 : 10
            anchors.rightMargin: incoming ? 10 : 40

            BorderImage {
                id: textBubble

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: messageText.bottom
                anchors.bottomMargin: -13

                visible: message != ""
                smooth: true
                source: incoming ? "../assets/bubble_right.png" : "../assets/bubble_left.png"
                border {top: 15; bottom: 40; left: incoming ? 15 : 21; right: incoming ? 21 : 15}
            }

            TextCustom {
                id: messageText

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: incoming ? 17 : 18 + 8
                anchors.rightMargin: 18 + 1
                anchors.top: parent.top
                anchors.topMargin: 12

                lineHeight: 1.3
                text: message
                wrapMode: Text.WordWrap
                fontSize: "medium"
                color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
            }

            Image {
                id: image

                property bool horizontal: image.sourceSize.width >= image.sourceSize.height

                width: horizontal ? parent.width : undefined
                height: horizontal ? undefined : 200
                clip: true
                fillMode: Image.PreserveAspectCrop
                source: imageSource
                smooth: true
                cache: false
                asynchronous: true
            }

            BorderImage {
                id: imageBubble

                anchors.fill: image
                anchors.topMargin: -4
                anchors.bottomMargin: -6
                anchors.leftMargin: incoming ? -5 : -12
                anchors.rightMargin: incoming ? -12 : -5

                visible: !textBubble.visible
                smooth: true
                source: incoming ? "../assets/bubble_image_right.png" : "../assets/bubble_image_left.png"
                border {top: 15; bottom: 40; left: incoming ? 15 : 21; right: incoming ? 21 : 15}
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
        anchors.topMargin: 10
        anchors.bottomMargin: 10
        contentWidth: parent.width
        contentHeight: messages.height
        spacing: 13
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
