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

/*
  Example usage:

  Flickable {
      id: flickable
  }

  Scrollbar {
      flickable: flickable
  }
*/
Scrollbar {
    id: scrollbar

    property Flickable flickable

    anchors.right: flickable.right
    anchors.top: flickable.top
    anchors.bottom: flickable.bottom

    pageSize: flickable.height
    contentSize: flickable.contentHeight


    // Synchronize scrollbar.contentPosition with flickable.contentY (aka. two-way property binding)
    function __contentYFromContentPosition(contentPosition) {
        return contentPosition
    }

    function __contentPositionFromContentY(contentY) {
        return contentY
    }

    function __updateContentPosition() {
        var newValue = __contentPositionFromContentY(flickable.contentY)
        if (scrollbar.contentPosition != newValue) {
            scrollbar.contentPosition = newValue
        }
    }

    function __updateContentY() {
        var newValue = __contentYFromContentPosition(scrollbar.contentPosition)
        if (flickable.contentY != newValue) {
            flickable.contentY = newValue
        }
    }

    Connections {
        target: flickable
        onContentYChanged: __updateContentPosition()
    }

    Connections {
        target: scrollbar
        onContentPositionChanged: __updateContentY()
    }
}
