import QtQuick 2
import "Widgets" as LocalWidgets
import Ubuntu.Components 0.1

Item {
    id: telephony
    width: singlePane ? 250 : 570
    height: 487

    state: appLayout
    property bool singlePane: state == "singlePane"
    property alias viewStack: rightPaneStacks.currentStack
    property alias view: rightPaneStacks.currentItem
    property alias selectedTabIndex: tabs.selectedTabIndex
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

    states: [
        State {
            name: "dualPane"

            PropertyChanges {
                target: leftPane
                parent: telephony
                width: 250
            }

            StateChangeScript {
                script: {
                    mainStack.clear();
                }
            }
        },

        State {
            name: "singlePane"

            StateChangeScript {
                script: {
                    mainStack.clear();
                    mainStack.push(leftPane);
                }
            }
        }
    ]

    function showLiveCall(clear) {
        if (clear) {
            resetView();
        }

        liveCall.load()
    }

    function showVoicemail() {
        resetView();
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

    function startChat(contactId, phoneNumber, clear) {
        var properties = { number: phoneNumber, newMessage: false };
        if (contactId) {
            properties["contactId"] = contactId;
        }
        if (clear) {
            resetView();
        }

        messages.load(properties);
    }

    function endCall() {
        if (liveCall.loaded) {
            viewStack.pop();
        }
    }

    function showContactDetails(contact, clear) {
        var properties = { contact: contact, added: false }
        if (clear) {
            resetView();
        }

        contactDetails.load(properties)
    }

    function showContactDetailsFromId(contactId) {
        contactModel.loadContactFromId(contactId);
    }

    function createNewContact() {
        resetView();
        contactDetails.load()
        view.createNewContact()
    }

    function startNewMessage() {
        resetView();
        messages.load({ newMessage: true })
    }

    function showKeypad() {
        resetView();
        keypad.load()
    }

    function showCallLog() {
        resetView();
        callLog.load()
    }

    function resetView() {
        if (singlePane) {
            while (viewStack.depth > 1) {
                viewStack.pop();
            }
        } else {
            viewStack.clear();
        }
    }

    PageStack {
        id: mainStack
        anchors.fill: parent
        visible: singlePane
    }

    Item {
        id: leftPane
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: singlePane ? parent.right : undefined
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
            anchors.bottom: leftPane.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            buttonsExpanded: true

            Tab {
                iconSource: (tabs.selectedTabIndex != 0) ? "assets/tab_icon_call_inactive.png" : "assets/tab_icon_call_active.png"
                page: Qt.resolvedUrl("PanelCalls/CallPanel.qml")
                property string pane: "Panes/CallEndedPane.qml"
            }

            Tab {
                iconSource: (tabs.selectedTabIndex != 1) ? "assets/tab_icon_messaging_inactive.png" : "assets/tab_icon_messaging_active.png"
                page: Qt.resolvedUrl("PanelMessages/MessagesPanel.qml")
                property string pane: "Panes/SelectMessagePane.qml"
            }

            Tab {
                iconSource: (tabs.selectedTabIndex != 2) ? "assets/tab_icon_contacts_inactive.png" : "assets/tab_icon_contacts_active.png"
                page: Qt.resolvedUrl("PanelContacts/ContactsPanel.qml")
                property string pane: "Panes/SelectContactPane.qml"
            }
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
            Behavior on anchors.bottomMargin { LocalWidgets.StandardAnimation {}}

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
        visible: !singlePane

        Rectangle {
            anchors.fill: parent
            color: "#ebebeb"
        }

        /* Instantiate a PageStack per tab and keep its loaded content alive.
           That makes the application stateful.
           Ref.: https://bugs.launchpad.net/newyork/+bug/1017659
        */
        Item {
            id: rightPaneStacks

            property variant currentStack: singlePane ? mainStack : children[tabs.selectedTabIndex]
            property variant currentItem: singlePane ? mainStack.currentPage :
                                                       (currentStack != undefined ? currentStack.currentPage : undefined)
            anchors.fill: parent
        }

        Repeater {
            model: tabs.children
            delegate: PageStack {
                id: stack
                property string source: modelData.pane
                property bool isCurrent: index == tabs.selectedTabIndex
                anchors.fill: parent
                visible: isCurrent
                onSourceChanged: {
                    stack.push(Qt.resolvedUrl(source))
                }

                Component.onCompleted: {
                    stack.push(Qt.resolvedUrl(source))
                }
            }
            onItemAdded: item.parent = rightPaneStacks
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
