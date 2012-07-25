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
    Connections {
        target: flickable
        onContentYChanged: if (flickable.contentY != scrollbar.contentPosition) {
                               scrollbar.contentPosition = flickable.contentY
                           }
    }
    Connections {
        target: scrollbar
        onContentPositionChanged: if (flickable.contentY != scrollbar.contentPosition) {
                                      flickable.contentY = scrollbar.contentPosition
                                  }
    }
}
