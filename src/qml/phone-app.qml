/*
 * Copyright 2012-2013 Canonical Ltd.
 *
 * This file is part of phone-app.
 *
 * phone-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * phone-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import "Widgets" as LocalWidgets
import Ubuntu.Components 0.1
import Ubuntu.HUD 1.0 as HUD
import Ubuntu.PhoneApp 0.1
import "PanelContacts"
import "PanelCommunications"
import "PanelDialer"

MainView {
    id: mainView

    automaticOrientation: true
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

    signal applicationReady

    onContactIdChanged: {
        if (contactId != "") {
            console.log("ContactId: " + contactId);
            contactModel.loadContactFromId(contactId);
        }
    }

    onSinglePaneChanged: {
        if (singlePane) {
            mainStack.clear();
            mainStack.push(leftPane);
        } else {
            mainStack.clear();
            leftPane.parent = mainView;
        }
    }

    Connections {
        target: contactModel
        onContactLoaded: mainView.showContactDetails(contact);
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
        // switch to the dialer tab
        resetView();
        switchToTab(dialer.tab);
        view.dialNumber = number;
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

    function startChat(number) {
        var contact = contactModel.contactFromPhoneNumber(number);
        if(contact) {
            showCommunication("contactId", contact.id, number, contact.id);
        } else {
            showCommunication("phoneNumber", number, number);
        }
    }

    function showMessage(id) {
        // Show a message by its given ID
        // TODO: implement
    }

    function endCall() {
        var callStack = singlePane ? mainStack : rightPaneStacks.stacks[liveCall.tab]
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

    function createNewContact(number) {
        resetView();
        contactDetails.load()
        view.createNewContact(number)
    }

    function startNewMessage() {
        resetView();
        communication.load({ newMessage: true })
    }

    function sendMessage(number, message) {
        chatManager.sendMessage(number, message);
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
        width: singlePane ? undefined : units.gu(31)
        parent: singlePane ? mainStack : mainView

        Tabs {
            id: tabs
            anchors.fill: parent
            ItemStyle.class: singlePane ? "new-tabs" : "tabs"

            property variant tabPageItems: [ callsTab.page.item, communicationsTab.page.item, contactsTab.page.item ]
            Component.onCompleted: ItemStyle.style.swipeToSwitchTabs = true

            Tab {
                id: callsTab
                objectName: "callsTab"

                property string pane: "Panes/CallEndedPane.qml"
                property string panel: "PanelDialer/DialerView.qml"
                property bool isCurrent: tabs.selectedTabIndex == 0

                title: "Call"
                iconSource: isCurrent ? "assets/tab_icon_call_active.png" : "assets/tab_icon_call_inactive.png"
                page: Loader {
                    source: callsTab.panel
                    anchors.fill: parent
                }
            }

            Tab {
                id: communicationsTab
                objectName: "communicationsTab"
                title: "Conversations"
                iconSource: isCurrent ? "assets/tab_icon_messaging_active.png" : "assets/tab_icon_messaging_inactive.png"

                property string pane: "Panes/SelectMessagePane.qml"
                property string panel: "PanelCommunications/CommunicationsPanel.qml"
                property bool isCurrent: tabs.selectedTabIndex == 1

                page: Loader {
                    source: communicationsTab.panel
                    asynchronous: true
                    anchors.fill: parent
                }
            }

            Tab {
                id: contactsTab
                objectName: "contactsTab"

                property string pane: "Panes/SelectContactPane.qml"
                property string panel: "PanelContacts/ContactsPanel.qml"
                property bool isCurrent: tabs.selectedTabIndex == 2

                title: "Contacts"
                iconSource: isCurrent ? "assets/tab_icon_contacts_active.png" : "assets/tab_icon_contacts_inactive.png"
                page: Loader {
                    source: contactsTab.panel
                    asynchronous: true
                    anchors.fill: parent
                }
            }
        }

        Rectangle {
            id: border

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            width: units.dp(1)
            color: "white"
            opacity: 0.3
            visible: !singlePane
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
                if (mainView.singlePane) {
                    return false;
                }

                if (!callManager.hasCalls) {
                    return false
                } else {
                    if (isVoicemailActive() && !mainView.voicemail.loaded) {
                        return true
                    } else if (!isVoicemailActive() && !mainView.liveCall.loaded) {
                        return true
                    }
                }
                return false
            }

            onClicked: isVoicemailActive() ? mainView.showVoicemail() : mainView.showLiveCall()
        }
    }

    Item {
        id: rightPaneStacks

        property variant currentStack: singlePane ? mainStack : stacks[tabs.selectedTabIndex]
        property variant currentItem: {
            if (singlePane && currentStack.depth == 1) {
                return tabs.tabPageItems[tabs.selectedTabIndex];
            }

            if (currentStack != undefined) {
                return currentStack.currentPage;
            }
            return undefined;
        }
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
            property string source: callsTab.pane
            property bool isCurrent: tabs.selectedTabIndex == liveCall.tab
            anchors.fill: parent
            visible: isCurrent

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
            property string source: communicationsTab.pane
            property bool isCurrent: tabs.selectedTabIndex == communication.tab
            anchors.fill: parent
            visible: isCurrent

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
            property string source: contactsTab.pane
            property bool isCurrent: tabs.selectedTabIndex == contactDetails.tab
            anchors.fill: parent
            visible: singlePane || isCurrent

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
            mainView.applicationReady()
        }
    }

    Connections {
        target: callManager
        onForegroundCallChanged: {
            // if there is no call, or if the views are already loaded, do not continue processing
            if (!callManager.foregroundCall || mainView.voicemail.loaded || mainView.liveCall.loaded) {
                return;
            }

            var currentPage = singlePane ? mainView.view : rightPaneStacks.stacks[dialer.tab].currentPage
            if (currentPage.source == dialer.source) {
                currentPage.dialNumber = ""
            }
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

    HUD.HUD {
        applicationIdentifier: "phone-app" // this must match the .desktop file!
        HUD.Context {
            HUD.QuitAction {
                onTriggered: Qt.quit()
            }

            HUD.Action {
                label: "Compose"
                keywords: "New Message"
                onTriggered: mainView.startNewMessage()
            }
            HUD.Action {
                label: "Add"
                keywords: "New Contact"
                onTriggered: mainView.createNewContact()
            }
        }
    }
}
