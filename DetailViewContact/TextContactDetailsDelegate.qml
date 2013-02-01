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
import "DetailTypeUtilities.js" as DetailUtils

BaseContactDetailsDelegate {
    id: delegate

    function saveDetail() {
        if (detail && detailTypeInfo.displayField && editor.text.trim().length > 0) {
            console.log("SAVING " + detail + " " + detailTypeInfo.displayField + " > " + editor.text)
            detail[detailTypeInfo.displayField] = (editor.text) ? editor.text : ""
            return true;
        } else return false;
    }

    editingActive: editor.activeFocus
    onFocusRequested: editor.forceActiveFocus()

    Item {
        parent: readOnlyContentBox

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: childrenRect.height + units.gu(2)

        Label {
            id: subTypeText

            anchors.left: parent.left
            anchors.top: parent.top
            anchors.topMargin: units.gu(1)
            horizontalAlignment: Text.AlignLeft
            height: units.gu(3)
            verticalAlignment: Text.AlignVCenter
            text: detailTypeInfo.showSubtype ? DetailUtils.getDetailSubType(detail) : detailTypeInfo.name
            fontSize: "small"
            elide: Text.ElideRight
            color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
            style: Text.Raised
            styleColor: "white"
        }

        Label {
            id: value

            anchors.top: subTypeText.bottom
            anchors.left: subTypeText.left
            height: units.gu(3)
            fontSize: "large"
            verticalAlignment: Text.AlignBottom
            elide: Text.ElideRight
            color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
            style: Text.Raised
            styleColor: "white"
            text: (detail && detailTypeInfo.displayField) ? detail[detailTypeInfo.displayField] : ""
        }

    }

    TextContactDetailsEditor {
        id: editor
        parent: editableContentBox
        anchors.left: parent.left
        anchors.right: parent.right
        text: value.text
        focus: true

        onActiveFocusChanged: {
            if (activeFocus) {
                // request the view to scroll to make sure this entry is visible.
                delegate.scrollRequested();
            }
        }
    }
}
