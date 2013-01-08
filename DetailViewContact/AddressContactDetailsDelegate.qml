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

import QtQuick 2.0
import Ubuntu.Components 0.1
import "DetailTypeUtilities.js" as DetailUtils

BaseContactDetailsDelegate {
    id: delegate

    function saveDetail() {
        if (editor.street.trim().length == 0 && editor.locality.trim().length == 0 &&
            editor.region.trim().length == 0 && editor.postcode.trim().length == 0 &&
            editor.country.trim().length == 0) return false;

        if (detail) {
            detail.street = editor.street
            detail.locality = editor.locality
            detail.region = editor.region
            detail.postcode = editor.postcode
            detail.country = editor.country
            return true;
        } else return false;
    }

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
            height: units.gu(3)
            text: detailTypeInfo.showSubtype ? DetailUtils.getDetailSubType(detail) : detailTypeInfo.name
            fontSize: "small"
            verticalAlignment: Text.AlignVCenter
        }

        Label {
            id: formattedAddress

            anchors.top: subTypeText.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: units.gu(1)
            fontSize: "large"
            elide: Text.ElideRight
            wrapMode: Text.WordWrap

            /* Render the address in a single field format */
            function nonEmpty(item) { return item && item.length > 0 }
            text: [
                detail.street,
                [ [detail.locality, detail.region].filter(nonEmpty).join(", "),
                  detail.postcode
                ].filter(nonEmpty).join(" "),
                detail.country
              ].filter(nonEmpty).join("\n");
        }

    }

    AddressContactDetailsEditor {
        id: editor
        parent: editableContentBox
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        street: detail ? detail.street : ""
        locality: detail ? detail.locality : ""
        region: detail ? detail.region : ""
        postcode: detail ? detail.postcode : ""
        country: detail ? detail.country : ""

        contactModelItem: delegate.detail
        focus: true
    }
}
