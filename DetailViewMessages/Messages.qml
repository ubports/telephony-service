import QtQuick 2
import TelephonyApp 0.1
import "../Widgets" as LocalWidgets
import Ubuntu.Components 0.1

Item {
    id: messages
    property variant contact
    property string number

    clip: true

    Component.onCompleted: messagesList.positionViewAtEnd();

    Component {
        id: sectionDelegate

        Item {
            height: childrenRect.height + 13

            TextCustom {
                anchors.left: parent.left
                anchors.leftMargin: 18
                text: section
                fontSize: "small"
                elide: Text.ElideRight
                color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
                style: Text.Raised
                styleColor: "white"
            }
        }
    }

    Component {
        id: messageImageDelegate

        MessageBubbleImage {
            maximumWidth: messagesList.width - parent.anchors.leftMargin - parent.anchors.rightMargin
            maximumHeight: 200

            imageSource: parent.imageSource
            mirrored: !parent.incoming
        }
    }

    Component {
        id: messageTextDelegate

        MessageBubbleText {
            text: parent.message
            mirrored: !parent.incoming
        }
    }

    MessagesProxyModel {
        id: messagesProxyModel
        messagesModel: messageLogModel
        ascending: true;
        phoneNumber: messages.number
    }

    ListView {
        id: messagesList

        anchors.fill: parent
        anchors.topMargin: 10
        anchors.bottomMargin: 10
        spacing: 24
        /* Necessary to force the instantiation of all the delegates in order
           for contentHeight to be accurate. That is required for
           ScrollbarForListView to operate properly */
        cacheBuffer: 2147483647
        orientation: ListView.Vertical
        ListModel { id: messagesModel }
        // FIXME: references to runtime and fake model need to be removed before final release
        model: typeof(runtime) != "undefined" ? fakeMessagesModel : messagesProxyModel
        section.delegate: sectionDelegate
        section.property: "date"
        delegate: Loader {
            /* Workaround Qt bug http://bugreports.qt.nokia.com/browse/QTBUG-16057
               More documentation at http://bugreports.qt.nokia.com/browse/QTBUG-18011
            */
            property bool incoming: model.incoming
            property string imageSource: model.avatar
            property string message: model.message

            anchors.left: if (sourceComponent == messageTextDelegate) return parent.left
                          else return incoming ? parent.left : undefined
            anchors.right: if (sourceComponent == messageTextDelegate) return parent.right
                          else return incoming ? undefined : parent.right

            anchors.leftMargin: incoming ? 10 : 39
            anchors.rightMargin: incoming ? 39 : 10

            sourceComponent: message != "" ? messageTextDelegate : messageImageDelegate
        }
        highlightFollowsCurrentItem: true
        onCountChanged: {
            messagesList.positionViewAtEnd()
        }
    }

    LocalWidgets.ScrollbarForListView {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        view: messagesList
        workaroundSectionHeightBug: false
    }
}
