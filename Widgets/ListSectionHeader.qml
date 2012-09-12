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

Item {
    width: 100
    height: 16

    property alias text: label.text

    // FIXME: replace with proper asset which should have a 1px border at the top and the bottom
    BorderImage {
        anchors.fill: parent
        source: "artwork/section_header.png"
        border {left: 0; right: 0; top: 1; bottom: 1}
    }

    TextCustom {
        id: label

        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -1
        fontSize: "x-small"
        font.italic: true
        elide: Text.ElideRight
        color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
        style: Text.Raised
        styleColor: "white"
    }
}
