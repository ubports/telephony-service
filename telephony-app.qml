import QtQuick 1.1
import QtMobility.contacts 1.1
import TelephonyApp 0.1
import "ContactUtils"

Rectangle {
    id: telephony
    width: 800
    height: 600

    function startCallToContact(contact, number) {
        // To keep this simple we rely on the fact that setting source to a
        // local file will immadiately make the item availalable.
        rightPaneContent.source = "DetailViewLiveCall/LiveCall.qml"
        rightPaneContent.item.contact = contact
        rightPaneContent.item.number = number
        rightPaneContent.item.startCall()
    }

    function startCallToNumber(number) {
        rightPaneContent.source = "DetailViewLiveCall/LiveCall.qml"
        rightPaneContent.item.contact = null
        rightPaneContent.item.number = number
        rightPaneContent.item.startCall()
    }

    function callNumber(number) {
        rightPaneContent.source = "DetailViewLiveCall/LiveCall.qml"
        rightPaneContent.item.contact = null
        rightPaneContent.item.number = number
        callManager.startCall(number);
    }

    function startChat(contact, number) {
        rightPaneContent.source = "DetailViewMessages/MessagesView.qml"
        rightPaneContent.item.contact = contact
        rightPaneContent.item.number = number
        rightPaneContent.item.newMessage = false
    }

    function endCall(duration) {
        rightPaneContent.source = "Panes/CallEndedPane.qml"
        rightPaneContent.item.text = duration;
        rightPaneContent.item.postText = "";
    }

    function showContactDetails(contact) {
        rightPaneContent.source = "DetailViewContact/ContactDetails.qml"
        rightPaneContent.item.contact = contact
    }

    function createNewContact(contacts) {
        rightPaneContent.source = "DetailViewContact/ContactDetails.qml"
        rightPaneContent.item.createNewContact(contacts)
    }

    function startNewMessage() {
        rightPaneContent.source = "DetailViewMessages/MessagesView.qml"
        rightPaneContent.item.newMessage = true
    }

    function showDial() {
        rightPaneContent.source = "DetailViewKeypad/KeypadView.qml"
    }

    ContactLoader {
        id: contactLoader
        contactId: contactKey

        onContactLoaded: {
            // switch to the contacts tab
            tabs.currentTab = 2;

            // and load the contact details
            showContactDetails(contact)
        }
    }

    Item {
        id: leftPane
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        Tabs {
            id: tabs
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right

            Component.onCompleted: rightPaneContent.source = "Panes/CallEndedPane.qml";
            onCurrentTabChanged: {
                switch (tabs.currentTab) {
                case 0:
                    rightPaneContent.source = "Panes/CallEndedPane.qml";
                    break;
                case 1:
                    rightPaneContent.source = "Panes/SelectMessagePane.qml";
                    break;
                case 2:
                    rightPaneContent.source = "Panes/SelectContactPane.qml";
                    break;
                }
            }
        }

        width: 250

        Loader {
            id: leftPaneContent
            anchors.top: tabs.bottom
            anchors.bottom: leftPane.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            source: {
                switch (tabs.currentTab) {
                case 0:
                    "PanelCalls/CallPanel.qml"
                    break;
                case 1:
                    "PanelMessages/MessagesPanel.qml"
                    break;
                case 2:
                    "PanelContacts/ContactsPanel.qml"
                    break;
                }
            }
        }
    }

    Item {
        id: rightPane
        anchors.left: leftPane.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        Rectangle {
            anchors.fill: parent
            color: "#ebebeb"

            Loader {
                id: rightPaneContent
                anchors.fill: parent
            }
        }
    }

    Connections {
        target: chatManager
        onChatReady: {
            if (rightPaneContent.item.viewName != "messages"
                    || rightPaneContent.item.number != contactId) {
                rightPaneContent.source = ""
                startChat("", contactId)
            }
        }
        onMessageReceived: {
            if (rightPaneContent.item.viewName != "messages"
                    || rightPaneContent.item.number != contactId) {
                rightPaneContent.source = ""
                startChat("", contactId)
            }
            rightPaneContent.item.addMessage(message, false)
        }
    }

    Connections {
        target: callManager
        onCallReady: {
            startCallToNumber(contactId)
        }
    }
}
