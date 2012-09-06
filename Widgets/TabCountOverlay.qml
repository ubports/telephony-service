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

BorderImage {
    id: overlay

    property int count: 0
    property bool selected: false

    source: selected ? "../assets/tab_overlay_count_active.png" : "../assets/tab_overlay_count_inactive.png"
    border.left: 3
    border.right: 3
    border.top: 3
    border.bottom: 3
    visible: count > 0
    width: border.left + border.right + countText.paintedWidth
    height: Math.max(sourceSize.height, countText.paintedHeight)

    TextCustom {
        id: countText
        fontSize: "x-small"
        color: "white"
        anchors.fill: parent
        text: count
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }
}
