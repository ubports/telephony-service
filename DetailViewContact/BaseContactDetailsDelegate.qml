/*
 * Copyright 2012 Canonical Ltd.
 *
 * This file is part of telephony-app.
 *
 * telephony-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import "../Widgets" as LocalWidgets
import Ubuntu.Components 0.1
import "DetailTypeUtilities.js" as DetailUtils

FocusScope {
    id: contactDetailsItem

    /* For deleted items it's not enough to hide them, they will still take space in
       the layout. We also need to set the height to zero to make them completely go away.
       There is a 2 pixels vertical spacing between fields in edit mode.
    */
    height: (deleted) ? 0 : (((editable) ? editableGroup.height + units.dp(2) : readOnlyGroup.height) + bottomSeparatorLine.height - units.dp(2))
    opacity: (deleted) ? 0.0 : 1.0

    signal scrollRequested()

    state: "read"
    states: [
        State {
            name: "read"
        },
        State {
            name: "edit"
            when: contactDetailsItem.editable
        }
    ]
    transitions: Transition {
        LocalWidgets.StandardAnimation { property: "height" }
    }

    property variant detail
    property variant detailTypeInfo

    property bool editable: false
    property bool editingActive: false
    property bool added: false
    /* We need to keep track of the deleted state of a detail because it will be
       actually deleted from the model only when we save the contact, even if we
       have already called contact.removeDetail() on it. */
    property bool deleted: false

    property bool bottomSeparator: true

    signal clicked(string value)
    signal actionClicked(string value)
    signal deleteClicked()
    signal focusRequested()

    /* Internal properties, use by derived components */
    property variant readOnlyContentBox: readOnlyContentBox
    property variant editableContentBox: editableContentBox

    function save() {
        // First save the subType of the detail, then check if we are being
        // subclassed and if the subclass defines its own saving function, and
        // in that case call it

        if (subTypeEditor.selectedValue != "")
            DetailUtils.setDetailSubType(detail, subTypeEditor.selectedValue);

        if (saveDetail instanceof Function) return saveDetail();
        else return true;
    }

    ListView.onRemove: SequentialAnimation {
        PropertyAction { target: contactDetailsItem; property: "ListView.delayRemove"; value: true }
        NumberAnimation { target: contactDetailsItem; property: "opacity"; to: 0; duration: 250 }
        NumberAnimation { target: contactDetailsItem; property: "height"; to: 0; duration: 250 }
        PropertyAction { target: contactDetailsItem; property: "ListView.delayRemove"; value: false }
    }

    ListView.onAdd: SequentialAnimation {
        NumberAnimation { target: contactDetailsItem; property: "opacity"; from: 0; duration: 250 }
        NumberAnimation { target: contactDetailsItem; property: "height"; from: 0; duration: 250 }
    }

    Image {
        id: bottomSeparatorLine

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: visible ? units.dp(2) : 0
        source: "../Widgets/artwork/ListItemSeparator.png"
        visible: contactDetailsItem.bottomSeparator
    }

    Item {
        id: readOnlyGroup

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        // center contentBox vertically
        height: contentBox.height
        opacity: editable ? 0.0 : 1.0
        enabled: opacity > 0.0
        Behavior on opacity {LocalWidgets.StandardAnimation {}}

        Rectangle {
            id: itemHighlight
            visible: contentBox.pressed
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: actionBox.left
            color: "white"
            opacity: 0.7
        }

        AbstractButton {
            id: contentBox

            anchors.left: parent.left
            anchors.leftMargin: units.gu(2)
            anchors.right: actionBox.left
            anchors.rightMargin: units.gu(1)
            anchors.top: parent.top
            height: readOnlyContentBox.height

            onClicked: contactDetailsItem.clicked(contactDetailsItem.value);

            Item {
                id: readOnlyContentBox

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: childrenRect.height
            }
        }

        /*Rectangle {
            id: readOnlySeparator

            anchors.verticalCenter: parent.verticalCenter
            height: units.gu(6)
            anchors.right: actionBox.left
            width: units.dp(1)
            color: "black"
            opacity: 0.1
            visible: actionBox.visible
        }*/

        AbstractButton {
            id: actionBox

            width: units.gu(7)
            visible: detailTypeInfo.hasAction
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            onClicked: contactDetailsItem.actionClicked(contactDetailsItem.value);

            Rectangle {
                id: actionHighlight
                visible: actionBox.pressed
                anchors.fill: actionBox
                color: "white"
                opacity: 0.7
            }

            Image {
                anchors.centerIn: parent
                width: units.gu(3)
                sourceSize.width: width
                fillMode: Image.PreserveAspectFit

                source: (detailTypeInfo.actionIcon) ? detailTypeInfo.actionIcon : ""
            }
        }
    }

    Item {
        id: editableGroup

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: childrenRect.height
        opacity: editable ? 1.0 : 0.0
        enabled: opacity > 0.0
        Behavior on opacity {LocalWidgets.StandardAnimation {}}
        focus: editable

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: childrenRect.height

            ContactDetailSubTypeChooser {
                id: subTypeEditor

                anchors.top: parent.top
                anchors.left: parent.left
                anchors.leftMargin: units.gu(2)
                anchors.right: parent.right
                anchors.rightMargin: units.gu(2)
                detailTypeInfo: contactDetailsItem.detailTypeInfo
                detail: contactDetailsItem.detail
                visible: editable
                active: editingActive

                onClicked: contactDetailsItem.focusRequested()
            }

            Button {
                id: removeButton

                anchors.right: parent.right
                anchors.rightMargin: units.gu(1)
                anchors.verticalCenter: editableContentBox.verticalCenter
                width: units.gu(2)
                iconSource: "../assets/edit_contact_mode_remove.png"
                ItemStyle.class: "transparent-button"
                visible: editingActive
                enabled: visible

                onClicked: {
                    deleted = true;
                    deleteClicked();
                }
            }

            Item {
                id: editableContentBox

                anchors.top: subTypeEditor.bottom
                anchors.left: parent.left
                anchors.leftMargin: units.gu(2)
                anchors.right: removeButton.left
                anchors.rightMargin: units.gu(0.5)
                height: childrenRect.height
            }
        }
    }
}
