import QtQuick 1.1
import QtMobility.contacts 1.1
import TelephonyApp 0.1
import "ContactUtils"
import "Widgets"

Item {
    id: telephony
    width: 570
    height: 487

    property alias viewSource: rightPaneContent.source
    property alias view: rightPaneContent.item
    property QtObject call: callManager.foregroundCall

    // Inventory of all the views in the application
    property ViewModel liveCall: ViewModel {source: "DetailViewLiveCall/LiveCall.qml"}
    property ViewModel voicemail: ViewModel {source: "DetailViewVoicemail/Voicemail.qml"}
    property ViewModel messages: ViewModel {source: "DetailViewMessages/MessagesView.qml"}
    property ViewModel callEnded: ViewModel {source: "Panes/CallEndedPane.qml"}
    property ViewModel contactDetails: ViewModel {source: "DetailViewContact/ContactDetails.qml"}
    property ViewModel keypad: ViewModel {source: "DetailViewKeypad/KeypadView.qml"}
    property ViewModel callLog: ViewModel {source: "DetailViewCallLog/CallLog.qml"}

    signal applicationReady

    function showLiveCall() {
        liveCall.load()
        view.startCall()
    }

    function showVoicemail() {
        voicemail.load()
    }

    function isVoicemailActive() {
        if (call)
            return call.voicemail
        return false
    }

    function callNumber(number) {
        callManager.startCall(number);
    }

    function startChat(contact, number) {
        messages.load()
        view.contact = contact
        view.number = number
        view.newMessage = false
    }

    function endCall() {
        callEnded.load()
    }

    function showContactDetails(contact) {
        contactDetails.load()
        view.contact = contact
        view.added = false
    }

    function showContactDetailsFromId(contactId) {
        contactLoader.contactId = contactId;
        // the contact details will be loaded once the contact loads
    }

    function createNewContact(contacts) {
        contactDetails.load()
        view.contactsModel = contacts
        view.createNewContact()
    }

    function startNewMessage() {
        messages.load()
        view.newMessage = true
    }

    function showKeypad() {
        keypad.load()
    }

    function showCallLog() {
        callLog.load()
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

        Image {
            id: background

            anchors.fill: parent
            source: "assets/noise_tile.png"
            fillMode: Image.Tile
        }

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
        }

        Rectangle {
            id: separator

            anchors.top: tabs.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 1
            color: "white"
        }

        Loader {
            id: leftPaneContent

            anchors.top: separator.bottom
            anchors.bottom: leftPane.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            source: tabs.model[tabs.currentTab].panel
        }

        Rectangle {
            id: border

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            width: 1
            color: "white"
            opacity: 0.3
        }

        OnCallPanel {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            visible: {
                if (!callManager.hasCalls) { 
                    return false
                } else {
                    if (isVoicemailActive() && !telephony.voicemail.loaded) {
                        return true 
                    } else if (!isVoicemailActive() && !telephony.liveCall.loaded) {
                        return true
                    }
                }
                return false
            }

            onClicked: isVoicemailActive() ? telephony.showVoicemail() : telephony.showLiveCall()
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

        Image {
            id: rightPaneBorder

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            source: "assets/detailview_left_border.png"
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
        target: telepathyHelper
        onAccountReady: {
            telephony.applicationReady()
        }
    }

    Connections {
        target: callManager
        onCallReady: {
            if (isVoicemailActive()) {
                showVoicemail()
            } else {
                showLiveCall();
            }
        }
    }
}
