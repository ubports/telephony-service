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

FocusScope {
    id: editor

    property variant detail
    property bool expanded: false

    property alias prefix: prefix.value
    property alias firstName: firstName.value
    property alias middleName: middleName.value
    property alias lastName: lastName.value
    property alias suffix: suffix.value

    height: fields.height + 2 * fields.anchors.topMargin

    BorderImage {
        id: background

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: fields.height + 2 * fields.anchors.topMargin
        Behavior on height { LocalWidgets.StandardAnimation {}}

        source: "../assets/edit_contact_mode_box.png"
        border {left: units.dp(1); right: units.dp(1); top: units.dp(2); bottom: 0}
    }

    Column {
        id: fields

        anchors.top: background.top
        anchors.topMargin: units.gu(0.5)
        anchors.left: background.left
        anchors.leftMargin: units.gu(1)
        anchors.right: background.right
        anchors.rightMargin: units.gu(1)
        spacing: units.dp(2)

        move: Transition { LocalWidgets.StandardAnimation {property: "y"}}

        EditBoxName {
            id: prefix
            objectName: "prefix"

            anchors.left: parent.left
            anchors.right: parent.right
            value: (detail) ? detail.prefix : ""
            description: "Prefix"
            visible: editor.expanded
        }

        EditBoxName {
            id: firstName
            objectName: "firstName"

            anchors.left: parent.left
            anchors.right: parent.right
            value: (detail) ? detail.firstName : ""
            description: "First"
            focus: true
        }

        EditBoxName {
            id: middleName
            objectName: "middleName"

            anchors.left: parent.left
            anchors.right: parent.right
            value: (detail) ? detail.middleName : ""
            description: "Middle"
            visible: editor.expanded
        }

        EditBoxName {
            id: lastName
            objectName: "lastName"

            anchors.left: parent.left
            anchors.right: parent.right
            value: (detail) ? detail.lastName : ""
            description: "Last"
        }

        EditBoxName {
            id: suffix
            objectName: "suffix"

            anchors.left: parent.left
            anchors.right: parent.right
            value: (detail) ? detail.suffix : ""
            description: "Suffix"
            visible: editor.expanded
        }
    }

    AbstractButton {
        id: expandButton

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.rightMargin: units.gu(1)
        width: units.gu(3)

        Image {
            anchors.top: parent.top
            anchors.topMargin: units.gu(1)
            anchors.right: parent.right
            source: editor.expanded ? "../assets/edit_contact_dropup_arrow.png" : "../assets/edit_contact_dropdown_arrow.png"
        }

        onClicked: editor.expanded = !editor.expanded
    }
}

