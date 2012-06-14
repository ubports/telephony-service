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

AbstractButton {
    id: tabButton

    property int count
    property alias iconSource: icon.source
    property bool selected
    property bool isFirst
    property bool isLast

    anchors.top: parent.top
    anchors.bottom: parent.bottom
    width: tabs.width / count + (isLast ? 0 : 1) // FIXME: reference to tabs

    BorderImage {
        id: background

        anchors.fill: parent
        source: {
            if (isFirst) {
                return selected ? "artwork/TabLeftSelected.png" : "artwork/TabLeftUnselected.png"
            } else if (isLast) {
                return selected ? "artwork/TabRightSelected.png" : "artwork/TabRightUnselected.png"
            } else {
                return selected ? "artwork/TabMiddleSelected.png" : "artwork/TabMiddleUnselected.png"
            }
        }

        border { left: isFirst ? 9 : 1; top: isFirst || isLast ? 9 : 2; right: isLast ? 10 : 2; bottom: 0 } // FIXME: take into account isFirst, isLast
        horizontalTileMode: BorderImage.Stretch
        verticalTileMode: BorderImage.Stretch
    }

    Image {
        id: icon

        anchors.centerIn: parent
        anchors.horizontalCenterOffset: isLast ? 0 : isFirst ? 2 : -1
        anchors.verticalCenterOffset: 2
        source: selected ? modelData.iconSelected : modelData.iconUnselected
    }
}
