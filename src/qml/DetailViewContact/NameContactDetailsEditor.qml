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

    Column {
        id: fields

        anchors.top: parent.top
        anchors.topMargin: units.gu(0.5)
        anchors.left: parent.left
        anchors.leftMargin: units.gu(1)
        anchors.right: parent.right
        anchors.rightMargin: units.gu(1)
        spacing: units.dp(2)

        move: Transition { LocalWidgets.StandardAnimation {property: "y"}}

        EditBoxName {
            id: prefix
            objectName: "prefix"

            anchors.left: parent.left
            anchors.right: parent.right
            value: (detail) ? detail.prefix : ""
            description: i18n.tr("Prefix")
            visible: editor.expanded
        }

        EditBoxName {
            id: firstName
            objectName: "firstName"

            anchors.left: parent.left
            anchors.right: parent.right
            value: (detail) ? detail.firstName : ""
            description: i18n.tr("First")
            focus: true
        }

        EditBoxName {
            id: middleName
            objectName: "middleName"

            anchors.left: parent.left
            anchors.right: parent.right
            value: (detail) ? detail.middleName : ""
            description: i18n.tr("Middle")
            visible: editor.expanded
        }

        EditBoxName {
            id: lastName
            objectName: "lastName"

            anchors.left: parent.left
            anchors.right: parent.right
            value: (detail) ? detail.lastName : ""
            description: i18n.tr("Last")
        }

        EditBoxName {
            id: suffix
            objectName: "suffix"

            anchors.left: parent.left
            anchors.right: parent.right
            value: (detail) ? detail.suffix : ""
            description: i18n.tr("Suffix")
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

