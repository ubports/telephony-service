import QtQuick 1.1
import QtMobility.contacts 1.1
import TelephonyApp 0.1
import "ContactUtils"
import "Widgets"

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

    function showContactDetailsFromId(contactId) {
        contactLoader.contactId = contactId;
        // the contact details will be loaded once the contact loads
    }

    function startNewMessage() {
        rightPaneContent.source = "DetailViewMessages/MessagesView.qml"
        rightPaneContent.item.newMessage = true
    }

    function showDial() {
        rightPaneContent.source = "DetailViewKeypad/KeypadView.qml"
    }

    function showCallLog() {
        rightPaneContent.source = "DetailViewCallLog/CallLog.qml"
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
        width: 250

        Tabs {
            id: tabs
            anchors.top: parent.top
            anchors.topMargin: 7
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10

            model: [
                {
                    "iconUnselected": "../assets/tab_icon_call_inactive.png",
                    "iconSelected": "../assets/tab_icon_call_active.png",
                    "panel": "PanelCalls/CallPanel.qml",
                    "pane": "Panes/CallEndedPane.qml"
                },
                {
                    "iconUnselected": "../assets/tab_icon_messaging_inactive.png",
                    "iconSelected": "../assets/tab_icon_messaging_active.png",
                    "panel": "PanelMessages/MessagesPanel.qml",
                    "pane": "Panes/SelectMessagePane.qml"
                },
                {
                    "iconUnselected": "../assets/tab_icon_contacts_inactive.png",
                    "iconSelected": "../assets/tab_icon_contacts_active.png",
                    "panel": "PanelContacts/ContactsPanel.qml",
                    "pane": "Panes/SelectContactPane.qml"
                }
            ]

            Component.onCompleted: rightPaneContent.source = tabs.model[tabs.currentTab].pane
            onCurrentTabChanged: rightPaneContent.source = tabs.model[tabs.currentTab].pane

            Rectangle {
                anchors.bottom: parent.bottom
                anchors.bottomMargin: -1
                anchors.left: parent.left
                anchors.right: parent.right
                height: 1
                color: "white"
            }
        }

        Loader {
            id: leftPaneContent
            anchors.top: tabs.bottom
            anchors.bottom: leftPane.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            source: tabs.model[tabs.currentTab].panel
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
        onCallEnded: {
            if (rightPaneContent.item.viewName == "livecall" &&
                rightPaneContent.item.number == contactId) {
                rightPaneContent.item.endCall()
            }
        }

    }
}
