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
    id: name

    property variant detail
    property bool editable

    property bool valid: editor.firstName.trim().length > 0 ||
                         editor.middleName.trim().length > 0 ||
                         editor.lastName.trim().length > 0

    height: (editable) ? editor.height : text.paintedHeight

    function isNotEmptyString(string) {
        return (string && string.length != 0);
    }

    function formatDisplayLabel() {
        // Concatenate all the non empty strings
        return (detail) ?
           [detail.prefix, detail.firstName, detail.middleName, detail.lastName, detail.suffix].filter(isNotEmptyString).join(" ") :
           "";
    }

    function save() {
        detail.firstName = editor.firstName
        detail.middleName = editor.middleName
        detail.lastName = editor.lastName
        detail.prefix = editor.prefix
        detail.suffix = editor.suffix
        contact.displayLabel = formatDisplayLabel()
    }

    onEditableChanged: if (editable) {
       editor.firstName = detail.firstName
       editor.middleName = detail.middleName
       editor.lastName = detail.lastName
       editor.prefix = detail.prefix
       editor.suffix = detail.suffix
    }

    Label {
        id: text
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.rightMargin: units.gu(1)
        height: paintedHeight

        fontSize: "large"
        elide: Text.ElideRight
        color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
        style: Text.Raised
        styleColor: "white"

        opacity: !editable ? 1.0 : 0.0
        Behavior on opacity { LocalWidgets.StandardAnimation {}}
        text: (detail && detail.displayLabel && detail.displayLabel.length > 0) ? detail.displayLabel : formatDisplayLabel()
    }

    NameContactDetailsEditor {
        id: editor
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        opacity: editable ? 1.0 : 0.0
        Behavior on opacity { LocalWidgets.StandardAnimation {}}

        detail: name.detail
        focus: true
    }
}
