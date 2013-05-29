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
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem
import "../Widgets" as LocalWidgets

FocusScope {
    id: contactDetailsHeader

    property variant contact: null
    property variant editable: false
    property alias contactNameValid: name.valid
    property color backgroundColor: "transparent"
    signal favoriteSaved()

    width: parent.width
    // ensure that there is equal padding at the top and bottom of labelBox
    height: editable ? Math.max(labelBox.height + labelBox.anchors.topMargin * 2, units.gu(10)) : units.gu(12)
    Behavior on height { LocalWidgets.StandardAnimation {}}

    function save() {
        name.save()
    }

    Rectangle {
        id: background
        anchors.fill: parent
        color: backgroundColor
    }

    UbuntuShape {
        id: icon

        anchors.left: parent.left
        anchors.leftMargin: units.gu(2)
        anchors.top: parent.top
        anchors.topMargin: units.gu(2)
        width: units.gu(8)
        height: units.gu(8)

        image: Image {
            source: contact && contact.avatar != "" ? contact.avatar : "../assets/avatar-default.png"
            asynchronous: true
            fillMode: Image.PreserveAspectCrop
            // since we don't know if the image is portrait or landscape without actually reading it,
            // set the sourceSize to be the size we need plus 30% to allow cropping.
            sourceSize.width: width * 1.3
            sourceSize.height: height * 1.3
        }
    }

    Image {
        anchors.right: labelBox.left
        anchors.top: labelBox.top
        height: units.gu(2)
        width: units.gu(2)
        anchors.rightMargin: units.gu(1)
        source: contact.isFavorite ? "../assets/favorite-selected.png" : "../assets/favorite-unselected.png"

        MouseArea {
            anchors.centerIn: parent
            height: units.gu(3)
            width: units.gu(3)
            onClicked: {
                contact.isFavorite = !contact.isFavorite
                if (!editable) {
                    contactDetailsHeader.favoriteSaved()
                }
            }
        }
    }

    Item {
        id: labelBox

        anchors.left: icon.right
        anchors.leftMargin: units.gu(4)
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: units.gu(3)
        height: childrenRect.height

        NameContactDetails {
            id: name
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right

            editable: header.editable
            detail: (contact) ? contact.name : undefined
            focus: true
        }
    }

    ListItem.ThinDivider {
        id: bottomDividerLine
        anchors.bottom: parent.bottom
    }
}
