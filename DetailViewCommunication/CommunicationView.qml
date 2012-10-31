import QtQuick 2.0
import TelephonyApp 0.1
import "../Widgets" as LocalWidgets
import "../"
import Ubuntu.Components.ListItems 0.1 as ListItem

LocalWidgets.TelephonyPage {
    id: view
    property alias contact: contactWatcher.contact
    property alias number: contactWatcher.phoneNumber
    property alias contactId: contactWatcher.contactId
    property bool newMessage: false
    property alias filterProperty: conversationProxyModel.filterProperty
    property alias filterValue: conversationProxyModel.filterValue

    property string pendingMessage

    title: "Communication"
    ContactWatcher {
        id: contactWatcher
    }

    function updateActiveChat() {
        // acknowledge messages as read just when the view is visible
        if (visible) {
            chatManager.activeChat = number;
        } else {
            chatManager.activeChat = "";
        }
    }

    Connections {
        target: chatManager

        onChatReady: {
            if (!contactModel.comparePhoneNumbers(phoneNumber, number)) {
                return;
            }

            if (pendingMessage != "") {
                chatManager.sendMessage(number, pendingMessage);
                pendingMessage = "";
            }
        }
    }

    // make sure the text channel gets closed after chatting
    Component.onDestruction: chatManager.endChat(number);

    onVisibleChanged: updateActiveChat();

    onNewMessageChanged: {
        if (newMessage) {
            number = "";
            headerLoader.focus = true;
        } else footer.focus = true;
    }

    onNumberChanged: {
        // get the contact
        view.contact = contactModel.contactFromPhoneNumber(number);

        updateActiveChat();
    }

    ConversationProxyModel {
        id: conversationProxyModel
        conversationModel: conversationAggregatorModel
        //searchString: search.text
        ascending: false
        grouped: false
    }

    Item {
        id: background

        anchors.fill: parent

        Image {
            anchors.fill: parent
            source: "../assets/noise_tile.png"
            fillMode: Image.Tile
        }

        Rectangle {
            anchors.fill: parent
            color: "black"
            opacity: 0.05
        }
    }

    Component {
        id: newHeaderComponent

        NewMessageHeader {
            width: view.width

            onContactSelected: {
                view.number = number;
                view.newMessage = false;
            }

            onNumberSelected: {
                view.number = number;
                view.newMessage = false;
            }
        }
    }

    Component {
        id: headerComponent

        MessagesHeader {
            width: view.width
            contact: view.contact
            number: view.number
        }
    }

    Loader {
        id: headerLoader

        sourceComponent: view.newMessage ? newHeaderComponent : headerComponent
        anchors.top: parent.top
        onLoaded: item.focus = true
    }

    Image {
        anchors.top: messagesLoader.top
        anchors.bottom: footer.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        source: "../assets/right_pane_pattern.png"
        fillMode: Image.Tile
    }

    Loader {
        id: messagesLoader

        sourceComponent: view.newMessage ? undefined : conversationComponent
        anchors.top: headerLoader.bottom
        anchors.bottom: footer.top
    }

    Component {
        id: messageComponent
        MessageItemDelegate {
            id: messageItemDelegate
        }
    }

    Component {
        id: callComponent
        CallItemDelegate {
            id: callItemDelegate
        }
    }

    Component {
        id: conversationComponent
        ListView {
            width: view.width
            height: view.height - footer.height - headerLoader.height
            model: conversationProxyModel
            clip: true
            delegate: ListItem.Base {
                id: delegate
                anchors.left: parent.left
                anchors.right: parent.right
                showDivider: true
                __height: 58

                Loader {
                    property string contactId: model ? model.contactId : ""
                    property string contactAlias: model ? model.contactAlias : ""
                    property url contactAvatar: model ? model.contactAvatar : ""
                    property variant timestamp: model ? model.timestamp : null
                    property bool incoming: model ? model.incoming : false
                    property string itemType: model ? model.itemType : "none"
                    property QtObject item: model ? model.item : null
                    property variant events: model ? model.events : null
                    anchors.fill: parent
                    sourceComponent: {
                        switch (itemType) {
                        case "message":
                            messageComponent;
                            break;
                        case "call":
                            callComponent;
                            break;
                        }
                    }
                }
            }
        }
    }

    MessagesFooter {
        id: footer

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        focus: true
        validRecipient: (!view.newMessage || headerLoader.item.text.match("^[0-9+][0-9+-]*$") != null)

        onNewMessage: {
            // if the user didn't select a number from the new message header, just
            // use whatever is on the text field
            if (view.newMessage) {
                var phoneNumber = headerLoader.item.text;
                view.number = phoneNumber
                view.newMessage = false;
            }

            if (chatManager.isChattingToContact(number)) {
                chatManager.sendMessage(number, message);
            } else {
                view.pendingMessage = message;
                chatManager.startChat(number);
            }
        }
    }
}
