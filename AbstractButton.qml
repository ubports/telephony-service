/*
 * This file is part of unity-2d
 *
 * Copyright 2010-2011 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 1.0

FocusScope {
    property bool enabled: true

    property alias color: background.color
    //property alias borderColor: background.border.color
    //property alias borderWidth: background.border.width
    property color borderColor;
    property int borderWidth;

    /* Use to manually set the "pressed" state of the button. This is not
       necessary in the normal use case, but is useful when a child item eats
       the mouse events (e.g. a DragArea).
       This is a clumsy workaround for the lack of a MouseProxy element
       (see http://bugreports.qt.nokia.com/browse/QTBUG-13007). */
    property bool pressed: false

    property alias mouseOver: mouse_area.containsMouse

    signal clicked

    Accessible.role: Accessible.PushButton

    Rectangle {
        id: background
        radius: 10.0
        width: parent.width
        height: parent.height
        border.color: parent.borderColor
        border.width: parent.borderWidth
    } // background


    MouseArea {
        id: mouse_area

        /* FIXME: workaround double click bug
                  http://bugreports.qt.nokia.com/browse/QTBUG-12250 */
        property bool double_clicked: false

        enabled: parent.enabled
        hoverEnabled: parent.enabled
        anchors.fill: parent
        onClicked: {
            if(double_clicked)
                double_clicked = false
            else
                parent.clicked()
        }
        onDoubleClicked: {
            double_clicked = true
        }
    }

    state: {
        if(pressed || mouse_area.pressed)
            return "pressed"
        else if(activeFocus)
            return "selected"
        else if(mouse_area.containsMouse)
            return "hovered"
        else
            return "default"
    }

    Keys.onPressed: {
        if (event.key == Qt.Key_Return) {
            clicked()
            event.accepted = true;
        }
    }
}
