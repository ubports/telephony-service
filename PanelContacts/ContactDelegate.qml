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
import Ubuntu.Components.ListItems 0.1 as ListItem
import Ubuntu.Components 0.1

ListItem.Empty {
    width: (parent) ? parent.width : undefined
    __height: units.gu(8)
    showDivider: true
    property variant model

    UbuntuShape {
        id: avatar
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: units.gu(2)
        height: units.gu(6)
        width: units.gu(6)
        image: Image {
            anchors.fill: parent
            source: model.decoration != "" ? Qt.resolvedUrl(model.decoration) : "../assets/avatar-default.png"
            asynchronous: true
            fillMode: Image.PreserveAspectCrop
            // since we don't know if the image is portrait or landscape without actually reading it,
            // set the sourceSize to be the size we need plus 30% to allow cropping.
            sourceSize.width: width * 1.3
            sourceSize.height: height * 1.3
        }
    }

    Label {
        text: model.display
        anchors.top: avatar.top
        anchors.left: avatar.right
        anchors.leftMargin: units.gu(1)
        fontSize: "medium"
    }
}
