import QtQuick 2.0
import "Widgets" as LocalWidgets
import Ubuntu.Components 0.1
import TelephonyApp 0.1

Item {
    id: telephony

    width: singlePane ? units.gu(40) : units.gu(80)
    height: units.gu(71)

    state: appLayout
    property bool singlePane: state == "singlePane"
    property alias viewStack: rightPaneStacks.currentStack
    property alias view: rightPaneStacks.currentItem
    property alias selectedTabIndex: tabs.selectedTabIndex
    property QtObject call: callManager.foregroundCall

    property string contactId: contactKey

    property string pendingMessage: ""
    property string pendingNumber: ""

    // Inventory of all the views in the application
    property ViewModel liveCall: ViewModel {source: "DetailViewLiveCall/LiveCall.qml"; tab: 0 }
    property ViewModel voicemail: ViewModel {source: "DetailViewVoicemail/Voicemail.qml"; tab: 0 }
    property ViewModel communication: ViewModel {source: "DetailViewCommunication/CommunicationView.qml"; tab: 1 }
    property ViewModel callEnded: ViewModel {source: "Panes/CallEndedPane.qml"; tab: 0 }
    property ViewModel dialer: ViewModel {source: "PanelDialer/DialerView.qml"; tab: 0 }
    property ViewModel contactDetails: ViewModel {source: "DetailViewContact/ContactDetails.qml"; tab: 2 }
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

    Connections {
        target: chatManager

        onChatReady: {
            if (!contactModel.comparePhoneNumbers(phoneNumber, pendingNumber)) {
                return;
            }

            if (pendingMessage != "") {
                chatManager.sendMessage(pendingNumber, pendingMessage);
                pendingMessage = "";
                pendingNumber = "";
            }
        }
    }

    function switchToTab(index) {
        selectedTabIndex = index
    }

    function showLiveCall(clear) {
        liveCall.load({}, clear)
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
        var callStack = rightPaneStacks.stacks[dialer.tab]
        if (callStack.currentPage.source == dialer.source) {
            callStack.currentPage.dialNumber = ""
        }
        callManager.startCall(number);
    }

    function callVoicemail(number) {
        callNumber(callManager.voicemailNumber);
    }

    function showCommunication(prop, value, phoneNumber, id, clear) {
        var properties = { filterProperty: prop,
                           filterValue: value,
                           newMessage: false,
                           phoneNumber: phoneNumber,
                           contactId: id };

        communication.load(properties, clear);
    }

    function endCall() {
        var callStack = rightPaneStacks.stacks[liveCall.tab]
        if (callStack.currentPage.source == liveCall.source || callStack.currentPage.source == voicemail.source) {
            callStack.pop();
        }
    }

    function showContactDetails(contact, clear) {
        var properties = { contact: contact, added: false }
        contactDetails.load(properties, clear)
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
        communication.load({ newMessage: true })
    }

    function sendMessage(number, message) {
        if (chatManager.isChattingToContact(number)) {
            chatManager.sendMessage(number, message);
        } else {
            pendingMessage = message;
            pendingNumber = number;
            chatManager.startChat(number);
        }
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

    LocalWidgets.Tabs {
        id: tabs
        anchors.fill: leftPane
        parent: leftPane
        ItemStyle.delegate: LocalWidgets.SlidingTabsDelegate {}

        Tab {
            id: callsTab

            property string pane: "Panes/CallEndedPane.qml"
            property string panel: "PanelDialer/DialerView.qml"

            iconSource: (tabs.selectedTabIndex != 0) ? "assets/tab_icon_call_inactive.png" : "assets/tab_icon_call_active.png"
            page: Item {
                id: callsTabPage

                anchors.fill: parent

                Loader {
                    anchors.fill: parent
                    source: singlePane ? "" : Qt.resolvedUrl(callsTab.panel)

                    onStatusChanged: {
                        if (status == Loader.Ready) {
                            item.visible = true
                        }
                    }
                }
            }
        }

        Tab {
            id: communicationsTab
            iconSource: (tabs.selectedTabIndex != 1) ? "assets/tab_icon_messaging_inactive.png" : "assets/tab_icon_messaging_active.png"

            property string pane: "Panes/SelectMessagePane.qml"
            property string panel: "PanelCommunications/CommunicationsPanel.qml"

            page: Item {
                id: communicationsTabPage

                anchors.fill: parent

                Loader {
                    anchors.fill: parent
                    source: singlePane ? "" : Qt.resolvedUrl(communicationsTab.panel)

                    onStatusChanged: {
                        if (status == Loader.Ready) {
                            item.visible = true
                        }
                    }
                }
            }

        }

        Tab {
            id: contactsTab

            property string pane: "Panes/SelectContactPane.qml"
            property string panel: "PanelContacts/ContactsPanel.qml"

            iconSource: (tabs.selectedTabIndex != 2) ? "assets/tab_icon_contacts_inactive.png" : "assets/tab_icon_contacts_active.png"
            page: Item {
                id: contactsTabPage

                anchors.fill: parent

                Loader {
                    anchors.fill: parent
                    source: singlePane ? "" : Qt.resolvedUrl(contactsTab.panel)

                    onStatusChanged: {
                        if (status == Loader.Ready) {
                            item.visible = true
                        }
                    }
                }
            }
        }
    }

    // TODO: this indicator will be provided by the Tabs component
    Item {
        id: tabIndicator
        height: units.dp(3)
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        Rectangle {
            anchors.fill: parent
            color: "#000000"
        }

        Rectangle { 
            color: "#f37505"
            height: parent.height
            width: parent.width/3
            x: (parent.width/3)*(tabs.selectedTabIndex)
        }
        z: 2
    }

    LocalWidgets.ChromeBar {
        id: chromeBar

        buttonsModel: telephony.view.chromeButtons ? telephony.view.chromeButtons : null
        showChromeBar: telephony.view.showChromeBar
        showBackButton: telephony.viewStack.depth > 1
        onButtonClicked: telephony.view.chromeButtonClicked(buttonName)
        onBackButtonClicked: telephony.viewStack.pop()
    }

    Image {
        id: background

        anchors.fill: parent
        source: "assets/noise_tile.png"
        fillMode: Image.Tile
    }

    Item {
        id: leftPane

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: singlePane ? parent.right : undefined
        width: singlePane ? undefined : units.gu(31)

        Rectangle {
            id: border

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            width: units.dp(1)
            color: "white"
            opacity: 0.3
        }
    }

    OnCallPanel {
        anchors.left: leftPane.left
        anchors.right: leftPane.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: shown ? 0 : -height
        Behavior on anchors.bottomMargin { LocalWidgets.StandardAnimation {}}
        z: 1

        property bool shown
        shown: {
            if (telephony.singlePane) {
                return false;
            }

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

    Item {
        id: rightPaneStacks

        property variant currentStack: stacks[tabs.selectedTabIndex]
        property variant currentItem: (currentStack != undefined ? currentStack.currentPage : undefined)

        property variant stacks: [ callsStack, communicationsStack, contactsStack ]

        anchors.fill: parent
        parent: rightPane
        visible: !singlePane

        /* Instantiate a PageStack per tab and keep its loaded content alive.
           That makes the application stateful.
           Ref.: https://bugs.launchpad.net/newyork/+bug/1017659
        */

        PageStack {
            id: callsStack
            property string source: singlePane ? callsTab.panel : callsTab.pane
            property bool isCurrent: tabs.selectedTabIndex == liveCall.tab
            anchors.fill: parent
            visible: singlePane || isCurrent
            parent: singlePane ? callsTabPage : rightPaneStacks
            __showHeader: false

            onSourceChanged: {
                callsStack.clear();
                callsStack.push(Qt.resolvedUrl(source));
                callsStack.currentPage.source = Qt.resolvedUrl(source);
            }

            Component.onCompleted: {
                callsStack.clear();
                callsStack.push(Qt.resolvedUrl(source));
                callsStack.currentPage.source = Qt.resolvedUrl(source);
            }
        }

        PageStack {
            id: communicationsStack
            property string source: singlePane ? communicationsTab.panel : communicationsTab.pane
            property bool isCurrent: tabs.selectedTabIndex == communication.tab
            anchors.fill: parent
            visible: singlePane || isCurrent
            parent: singlePane ? communicationsTabPage : rightPaneStacks
            __showHeader: false

            onSourceChanged: {
                communicationsStack.clear();
                communicationsStack.push(Qt.resolvedUrl(source));
                communicationsStack.currentPage.source = Qt.resolvedUrl(source);
            }

            Component.onCompleted: {
                communicationsStack.clear();
                communicationsStack.push(Qt.resolvedUrl(source));
                communicationsStack.currentPage.source = Qt.resolvedUrl(source);
            }
        }

        PageStack {
            id: contactsStack
            property string source: singlePane ? contactsTab.panel : contactsTab.pane
            property bool isCurrent: tabs.selectedTabIndex == contactDetails.tab
            anchors.fill: parent
            visible: singlePane || isCurrent
            parent: singlePane ? contactsTabPage : rightPaneStacks
            __showHeader: false

            onSourceChanged: {
                contactsStack.clear();
                contactsStack.push(Qt.resolvedUrl(source));
                contactsStack.currentPage.source = Qt.resolvedUrl(source);
            }

            Component.onCompleted: {
                contactsStack.clear();
                contactsStack.push(Qt.resolvedUrl(source));
                contactsStack.currentPage.source = Qt.resolvedUrl(source);
            }
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

    Image {
        source: "assets/phone_grid.png"
        anchors.top: parent.top
        anchors.left: parent.left
        opacity: 0.0
    }
}
