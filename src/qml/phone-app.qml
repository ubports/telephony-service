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
    width: units.gu(40)
    height: units.gu(71)

    property alias viewStack: mainStack
    property variant view: {
        if (mainStack.depth == 1) {
            return tabs.tabPageItems[tabs.selectedTabIndex];
        }
        return mainStack.currentPage
    }

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

    Component.onCompleted: {
        i18n.domain = "phone-app"
        console.debug(i18n.tr("%1 second call", "%1 seconds call", 10).arg(10))
        mainStack.push(mainPane);
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
        var callStack = mainStack
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
        while (viewStack.depth > 1) {
            viewStack.pop();
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

        Item {
            id: mainPane

            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right

            Tabs {
                id: tabs
                anchors.fill: parent

                property variant tabPageItems: [ callsTab.page.item, communicationsTab.page.item, contactsTab.page.item ]

                Tab {
                    id: callsTab
                    objectName: "callsTab"

                    property string pane: "Panes/CallEndedPane.qml"
                    property string panel: "PanelDialer/DialerView.qml"
                    property bool isCurrent: tabs.selectedTabIndex == 0

                    title: i18n.tr("Call")
                    iconSource: isCurrent ? "assets/tab_icon_call_active.png" : "assets/tab_icon_call_inactive.png"
                    page: Loader {
                        source: callsTab.panel
                        anchors.fill: parent
                    }
                }

                Tab {
                    id: communicationsTab
                    objectName: "communicationsTab"
                    title: i18n.tr("Conversations")
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

                    title: i18n.tr("Contacts")
                    iconSource: isCurrent ? "assets/tab_icon_contacts_active.png" : "assets/tab_icon_contacts_inactive.png"
                    page: Loader {
                        source: contactsTab.panel
                        asynchronous: true
                        anchors.fill: parent
                    }
                }
            }
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

            var currentPage = mainView.view
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
                label: i18n.tr("Compose")
                keywords: i18n.tr("New Message")
                onTriggered: mainView.startNewMessage()
            }
            HUD.Action {
                label: i18n.tr("Add")
                keywords: i18n.tr("New Contact")
                onTriggered: mainView.createNewContact()
            }
        }
    }
}
