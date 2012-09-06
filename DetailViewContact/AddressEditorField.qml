/*
 * Copyright 2012 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 1.1
import Ubuntu.Components 0.1
import "../fontUtils.js" as FontUtils

FocusScope {
    id: addressEditorField

    property alias value: input.text
    property alias description: label.text

    height: childrenRect.height

    TextCustom {
        id: label

        anchors.left: parent.left
        anchors.right: parent.right

        fontSize: "large"
        color: Qt.rgba(0.7, 0.7, 0.7, 1.0)
        font.italic: true
        opacity: input.text == "" ? 1.0 : 0.0
    }

    TextInput {
        id: input

        anchors.left: parent.left
        anchors.right: parent.right

        font.pixelSize: FontUtils.sizeToPixels("large")
        color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
        selectByMouse: true
        focus: true
    }
}
