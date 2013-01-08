/*
 * Copyright 2012 Canonical Ltd.
 *
 * This file is part of telephony-app.
 *
 * telephony-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import "../Widgets" as LocalWidgets
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem

Item {
    id: messageList

    property alias model: listView.model

    Component {
        id: delegateComponent
        CommunicationDelegate {
            item: (model && model.item) ? model.item : null
            title: (model && model.contactAlias) ? model.contactAlias : ""
            //subtitle: "(TODO: show phone type)"
            text: (model && model.item && model.item.message) ? model.item.message : ""
            timestamp: (model && model.timestamp) ? model.timestamp : null
            avatar: (model && model.contactAvatar) ? model.contactAvatar : ""
            itemIcon:  {
                switch (model.itemType) {
                case "message":
                    "../assets/messages.png";
                    break;
                case "call":
                    if (item.missed) {
                        "../assets/missed-call.png"
                    } else if (item.incoming) {
                        "../assets/incoming-call.png"
                    } else {
                        "../assets/outgoing-call.png"
                    }
                    break;
                case "group":
                    "../assets/tab_icon_contacts_inactive.png";
                    break;
                default:
                    "";
                    break;
                }
            }
            selected: isSelected()

            function isSelected() {
                if (!model || !model.groupingProperty) {
                    return false;
                }

                if (!telephony.view || !telephony.view.filterProperty) {
                    return false;
                }

                return (telephony.view.filterProperty == model.groupingProperty) &&
                        (telephony.view.filterValue == model.item[model.groupingProperty]);
            }

            onClicked: {
                telephony.showCommunication(model.groupingProperty, model.item[model.groupingProperty], "", model.contactId, true);
            }
        }
    }

    ListView {
        id: listView
        anchors.fill: parent
        clip: true
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
        flickableItem: listView
        align: Qt.AlignTrailing
        __interactive: false
    }
}
