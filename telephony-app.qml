import QtQuick 1.1
import QtMobility.contacts 1.1
import "Widgets"

Item {
    id: telephony
    width: 570
    height: 487

    property alias viewLoader: rightPaneLoaders.currentLoader
    property alias view: rightPaneLoaders.currentItem
    property alias currentTab: tabs.currentTab
    property QtObject call: callManager.foregroundCall

    property string contactId: contactKey

    // Inventory of all the views in the application
    property ViewModel liveCall: ViewModel {source: "DetailViewLiveCall/LiveCall.qml"; tab: 0 }
    property ViewModel voicemail: ViewModel {source: "DetailViewVoicemail/Voicemail.qml"; tab: 0 }
    property ViewModel messages: ViewModel {source: "DetailViewMessages/MessagesView.qml"; tab: 1 }
    property ViewModel callEnded: ViewModel {source: "Panes/CallEndedPane.qml"; tab: 0 }
    property ViewModel contactDetails: ViewModel {source: "DetailViewContact/ContactDetails.qml"; tab: 2 }
    property ViewModel keypad: ViewModel {source: "DetailViewKeypad/KeypadView.qml"; tab: 0 }
    property ViewModel callLog: ViewModel {source: "DetailViewCallLog/CallLog.qml"; tab: 0 }

    signal applicationReady

    onContactIdChanged: {
        if (contactId != "") {
            console.log("ContactId: " + contactId);
            contactModel.loadContactFromId(contactId);
        }
    }

    Connections {
        target: contactModel
        onContactLoaded: telephony.showContactDetails(contact);
    }

    function showLiveCall() {
        liveCall.load()
    }

    function showVoicemail() {
        voicemail.load()
    }

    function isVoicemailActive() {
        if (call) {
            return call.voicemail;
        } else {
            return false
        }
    }

    function callNumber(number) {
        callManager.startCall(number);
    }

    function startChat(customId, number, threadId) {
        messages.load()
        view.number = number
        view.customId = customId
        view.threadId = threadId
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
        contactModel.loadContactFromId(contactId);
    }

    function createNewContact() {
        contactDetails.load()
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

    function resetView() {
        viewLoader.source = tabs.model[tabs.currentTab].pane
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
                    "pane": "Panes/CallEndedPane.qml",
                    "eventCountSource": "0"
                },
                {
                    "iconUnselected": "../assets/tab_icon_messaging_inactive.png",
                    "iconSelected": "../assets/tab_icon_messaging_active.png",
                    "panel": "PanelMessages/MessagesPanel.qml",
                    "pane": "Panes/SelectMessagePane.qml",
                    "eventCountSource": "chatManager.unreadMessagesCount"
                },
                {
                    "iconUnselected": "../assets/tab_icon_contacts_inactive.png",
                    "iconSelected": "../assets/tab_icon_contacts_active.png",
                    "panel": "PanelContacts/ContactsPanel.qml",
                    "pane": "Panes/SelectContactPane.qml",
                    "eventCountSource": "0"
                }
            ]
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
            anchors.bottomMargin: shown ? 0 : -height
            Behavior on anchors.bottomMargin {StandardAnimation {}}

            property bool shown
            shown: {
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
        }

        /* Instantiate a Loader per tab and keep its loaded content alive.
           That makes the application stateful.
           Ref.: https://bugs.launchpad.net/newyork/+bug/1017659
        */
        Item {
            id: rightPaneLoaders

            property variant currentLoader: children[children.length - 1 - tabs.currentTab]
            property variant currentItem: currentLoader != undefined ? currentLoader.item : undefined
            anchors.fill: parent
        }

        Repeater {
            model: tabs.model
            delegate: Loader {
                property bool isCurrent: index == tabs.currentTab
                parent: rightPaneLoaders
                anchors.fill: parent
                source: modelData.pane
                visible: isCurrent
                focus: isCurrent

                onLoaded: item.focus = isCurrent
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
            application.activateWindow();
        }
    }
}
