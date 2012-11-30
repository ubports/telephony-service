import QtQuick 2.0
import TelephonyApp 0.1
import "../Widgets" as LocalWidgets
import "../"
import "../dateUtils.js" as DateUtils
import Ubuntu.Components.ListItems 0.1 as ListItem
import Ubuntu.Components 0.1

LocalWidgets.TelephonyPage {
    id: view
    objectName: "communicationView"
    property alias contact: contactWatcher.contact
    property alias number: contactWatcher.phoneNumber
    property alias contactId: contactWatcher.contactId
    property bool newMessage: false
    property alias filterProperty: conversationProxyModel.filterProperty
    property alias filterValue: conversationProxyModel.filterValue
    property string phoneNumber: ""

    property string pendingMessage

    title: "Communication"

    chromeButtons: newMessage ? null : buttons

    ListModel {
        id: buttons

        ListElement {
            label: "Edit"
            name: "edit"
        }
    }

    onChromeButtonClicked: {
        if (buttonName == "edit") {
            console.log("TODO: edit clicked, implement");
        }
    }

    ContactWatcher {
        id: contactWatcher
    }

    function updateActiveChat() {
        // acknowledge messages as read just when the view is visible
        // FIXME: check how to implement that in the new layout.
    }

    Connections {
        target: chatManager

        onChatReady: {
            if (!contactModel.comparePhoneNumbers(phoneNumber, view.phoneNumber)) {
                return;
            }

            if (pendingMessage != "") {
                chatManager.sendMessage(view.phoneNumber, pendingMessage);
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
            title: filterValue
        }
    }

    Loader {
        id: headerLoader

        sourceComponent: view.newMessage ? newHeaderComponent : null
        anchors.top: parent.top
        onLoaded: item.focus = true

        height: sourceComponent != null ? childrenRect.height : 0
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

    }

    ListView {
        id: listView
        anchors.top: headerLoader.bottom
        anchors.bottom: footer.top
        anchors.left: parent.left
        anchors.right: parent.right
        model: conversationProxyModel
        clip: true
        header: headerComponent

        section.property: "timeSlot"
        section.delegate: Column {
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: units.gu(0.5)


            ListItem.ThinDivider {
                height: units.gu(0.5)
            }

            Label {
                anchors.left: parent.left
                anchors.leftMargin: units.gu(2)
                fontSize: "small"
                elide: Text.ElideRight
                color: "#333333"
                opacity: 0.6
                text: DateUtils.formatFriendlyDate(section);
                height: paintedHeight + units.gu(1)
            }
        }


        delegate: CommunicationDelegate {
            id: communicationDelegate

            itemType: model.itemType
            incoming: model.incoming
            missed: model.item.missed ? model.item.missed : false
            message: model.item.message ? model.item.message : ""
            item: model.item
            itemIcon: {
                switch (model.itemType) {
                case "message":
                    "../assets/contact_icon_message.png";
                    break;
                case "call":
                    "../assets/contact_icon_phone.png";
                    break;
                case "group":
                    "../assets/tab_icon_contacts_inactive.png";
                    break;
                default:
                    "";
                    break;
                }
            }

            onClicked: {
                if (itemType == "call") {
                    telephony.callNumber(item.phoneNumber);
                } else {
                    view.phoneNumber = item.phoneNumber
                }
            }
        }
    }

    Scrollbar {
        flickableItem: listView
        align: Qt.AlignTrailing
    }

    MessagesFooter {
        id: footer

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: visible ? units.gu(5) : 0
        visible: view.phoneNumber != "" || view.newMessage == true
        focus: true
        validRecipient: (!view.newMessage || headerLoader.item.text.match("^[0-9+][0-9+-]*$") != null)

        onNewMessage: {
            // if the user didn't select a number from the new message header, just
            // use whatever is on the text field
            if (view.newMessage) {
                var phoneNumber = headerLoader.item.text;
                view.number = phoneNumber
                view.phoneNumber = phoneNumber
                view.newMessage = false;
            }

            if (chatManager.isChattingToContact(view.phoneNumber)) {
                chatManager.sendMessage(view.phoneNumber, message);
            } else {
                view.pendingMessage = message;
                chatManager.startChat(view.phoneNumber);
            }
        }
    }
}
