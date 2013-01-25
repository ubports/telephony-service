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
import Ubuntu.Components 0.1

FocusScope {
    id: addressEditorField

    property alias value: input.text
    property alias description: label.text

    height: childrenRect.height

    Label {
        id: label

        anchors.left: parent.left
        anchors.right: parent.right

        fontSize: "medium"
        color: Qt.rgba(0.7, 0.7, 0.7, 1.0)
        font.italic: true
        opacity: input.text == "" ? 1.0 : 0.0
    }

    TextInput {
        id: input

        anchors.left: parent.left
        anchors.right: parent.right

        font.pixelSize: FontUtils.sizeToPixels("medium")
        color: activeFocus? "#dd4814" : Qt.rgba(0.4, 0.4, 0.4, 1.0)
        selectByMouse: true
        focus: true
    }
}
