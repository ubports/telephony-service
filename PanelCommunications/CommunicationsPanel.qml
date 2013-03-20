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
import Ubuntu.PhoneApp 0.1
import "../Widgets" as LocalWidgets
import Ubuntu.Components.ListItems 0.1 as ListItem
import Ubuntu.Components 0.1

LocalWidgets.PhonePage {
    id: messageList
    objectName: "communicationPanel"
    title: "Conversations"
    tools: ToolbarActions {
        Action {
            iconSource: Qt.resolvedUrl("../assets/compose.png")
            text: "Compose"
            onTriggered: mainView.startNewMessage()
        }
    }

    property alias searchTerm: conversationProxyModel.searchString

    ConversationProxyModel {
        id: conversationProxyModel
        conversationModel: conversationAggregatorModel
        ascending: false
        grouped: true
        showLatestFromGroup: true
    }

    Component {
        id: delegateComponent
        CommunicationDelegate {

            selected: isSelected()

            function isSelected() {
                if (!model || !model.groupingProperty) {
                    return false;
                }

                if (!mainView.view || !mainView.view.filterProperty) {
                    return false;
                }

                return (mainView.view.filterProperty == model.groupingProperty) &&
                        (mainView.view.filterValue == model.item[model.groupingProperty]);
            }

            onClicked: {
                mainView.showCommunication(model.groupingProperty, model.item[model.groupingProperty], "", model.contactId, true);
            }
        }
    }

    ListView {
        id: communicationsList

        anchors.top: parent.top
        anchors.topMargin: units.gu(1)
        anchors.bottom: keyboard.top
        anchors.left: parent.left
        anchors.right: parent.right
        clip: true
        model: conversationProxyModel
        cacheBuffer: height * 2
        currentIndex: -1

        header: Column {
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: units.gu(1)

            TextField {
                id: search
                objectName: "messageSearchBox"

                anchors.left: parent.left
                anchors.leftMargin: units.gu(1)
                anchors.right: parent.right
                anchors.rightMargin: units.gu(1)

                //placeholderText: "Search"
                Keys.onEscapePressed: text = ""
                height: units.gu(4)

                primaryItem: AbstractButton {
                    width: units.gu(3)
                    Image {
                        anchors.left: parent.left
                        anchors.leftMargin: units.gu(0.5)
                        anchors.verticalCenter: parent.verticalCenter
                        source: "../assets/search.png"
                    }
                    onClicked: search.text = ""
                }

                Binding {
                    target: messageList
                    property: "searchTerm"
                    value: search.text
                }
            }

            ListItem.ThinDivider {
                visible: !mainView.singlePane
            }

            ListItem.Standard {
                id: newMessage
                anchors.left: parent.left
                anchors.right: parent.right
                height: visible ? units.gu(4) : 0
                visible: !mainView.singlePane
                __leftIconMargin: units.gu(2)
                __rightIconMargin: units.gu(2)

                icon: Qt.resolvedUrl("../assets/add_new_message_icon.png")
                iconFrame: false
                text: "New Message"
                onClicked: mainView.startNewMessage()
                selected: mainView.communication.loaded && mainView.view.newMessage
            }
        }

        // We are not using sections right now, re-enable that in the future in case the
        // design changes.
        //section.property: "date"
        //section.delegate: ListItem.Divider { }
        delegate: Loader {
            id: conversationLoader
            sourceComponent: delegateComponent
            asynchronous: true
            anchors.left: parent.left
            anchors.right: parent.right
            height: item ? item.height : units.gu(8.5)

            Binding {
                target: conversationLoader.item
                property: "model"
                value: model
                when: conversationLoader.status == Loader.Ready
            }
        }
    }

    Scrollbar {
        flickableItem: communicationsList
        align: Qt.AlignTrailing
        __interactive: false
    }

    LocalWidgets.KeyboardRectangle {
        id: keyboard
    }
}
