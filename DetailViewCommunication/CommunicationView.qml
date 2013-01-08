/*
 * Copyright 2012 Canonical Ltd.
 *
 * This file is part of telephony-app.
 *
 * telephony-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

    Component.onCompleted: {
        if (view.newMessage) {
            headerLoader.focus = true;
            headerLoader.forceActiveFocus()
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

    Component {
        id: newHeaderComponent

        NewMessageHeader {
            width: view.width

            onContactSelected: {
                view.filterProperty = "contactId";
                view.filterValue = contact.id;
                view.number = number;
                view.phoneNumber = number;
                view.newMessage = false;
            }

            onNumberSelected: {
                view.filterProperty = "phoneNumber"
                view.filterValue = number;
                view.number = number;
                view.phoneNumber = number;
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

    ListView {
        id: listView
        anchors.top: headerLoader.bottom
        anchors.bottom: footer.top
        anchors.left: parent.left
        anchors.right: parent.right
        model: view.newMessage ? null : conversationProxyModel
        clip: true
        header: view.newMessage ? null: headerComponent

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
                height: paintedHeight + units.gu(2)
                verticalAlignment: Text.AlignVCenter
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
                    "../assets/messages.png";
                    break;
                case "call":
                    "../assets/phone-call.png";
                    break;
                case "group":
                    "../assets/contact.png";
                    break;
                default:
                    "";
                    break;
                }
            }

            onClicked: {
                listView.currentIndex = index
                if (itemType == "call") {
                    telephony.callNumber(item.phoneNumber);
                } else {
                    view.phoneNumber = item.phoneNumber
                    footer.focus = true
                }
            }
        }

        onHeightChanged: {
            listView.positionViewAtIndex(listView.currentIndex, ListView.End)
        }

        onCurrentIndexChanged: {
            listView.positionViewAtIndex(listView.currentIndex, ListView.End)
        }
    }

    Scrollbar {
        flickableItem: listView
        align: Qt.AlignTrailing
        __interactive: false


    }

    MessagesFooter {
        id: footer

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: keyboard.top
        visible: view.phoneNumber != "" || view.newMessage == true || footer.focus == true
        focus: false
        onFocusChanged: {
            if (!focus) view.phoneNumber = ""
        }
        validRecipient: (!view.newMessage || headerLoader.item.text.match("^[0-9+][0-9+-]*$") != null)
        newConversation: view.newMessage

        onNewMessage: {
            // if the user didn't select a number from the new message header, just
            // use whatever is on the text field
            if (view.newMessage) {
                var phoneNumber = headerLoader.item.text;
                view.filterProperty = "phoneNumber"
                view.filterValue = phoneNumber;
                view.number = phoneNumber;
                view.phoneNumber = phoneNumber;
                view.newMessage = false;
            }

            if (chatManager.isChattingToContact(view.phoneNumber)) {
                chatManager.sendMessage(view.phoneNumber, message);
            } else {
                view.pendingMessage = message;
                chatManager.startChat(view.phoneNumber);
            }

            listView.currentIndex = 0;
            listView.positionViewAtIndex(listView.currentIndex, ListView.Begin);
        }
    }

    LocalWidgets.KeyboardRectangle {
        id: keyboard
    }
}
