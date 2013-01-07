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

Button {
    id: button

    property alias iconSource: icon.source
    property bool selected: false
    ItemStyle.class: "transparent"

    width: units.gu(9)
    height: units.gu(9)

    property int iconWidth: 0
    property int iconHeight: 0

    BorderImage {
        anchors.fill: parent
        source: (selected || pressed) ? "../assets/dialer_pad_bg_pressed.png" : "../assets/dialer_pad_bg.png"
    }

    Image {
        id: icon
        anchors.centerIn: parent
        anchors.horizontalCenterOffset: units.dp(1)
        width: (iconWidth > 0) ? iconWidth : undefined
        height: (iconHeight > 0) ? iconHeight : undefined
        source: icon
        fillMode: Image.PreserveAspectFit
    }
}
