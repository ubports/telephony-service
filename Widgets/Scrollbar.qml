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

// FIXME:
// - add dragging
Item {
    id: scrollbar

    property variant targetFlickable
    property bool __scrollable: targetFlickable.visibleArea.heightRatio != 1.0

    width: 30
    height: 400

    opacity: __scrollable ? 1.0 : 0.0
    Behavior on opacity {NumberAnimation {duration: 100; easing.type: Easing.InOutQuad}}

    Rectangle {
        id: slider

        property int minimalHeight: 40

        anchors.right: parent.right
        width: 2
        color: "#fc7134"

        height: Math.min(scrollbar.height, Math.max(minimalHeight, targetFlickable.visibleArea.heightRatio * scrollbar.height))
        Behavior on height {NumberAnimation {duration: 200; easing.type: Easing.InOutQuad}}

        Binding {
            target: slider
            property: "y"
            value: {
                var clampedYPosition = Math.max(0, Math.min(1-targetFlickable.visibleArea.heightRatio,
                                                            targetFlickable.visibleArea.yPosition))
                return clampedYPosition * scrollbar.height
            }
            when: !dragMouseArea.drag.active
        }
    }

    Rectangle {
        id: sliderThumbConnector

        anchors.right: parent.right
        width: slider.width

        property int thumbMargin: 3
        property bool isTop: thumb.y > slider.y // FIXME: is that correct?

        // FIXME: simplify
        y: isTop ? slider.y + slider.height : thumb.y + thumbMargin
        height: isTop ? (thumb.y + thumb.height - thumbMargin) - y : (slider.y) - y

        color: "white"
        opacity: thumb.shown ? 1.0 : 0.0
        Behavior on opacity {NumberAnimation {duration: 100; easing.type: Easing.InOutQuad}}
    }

    MouseArea {
        id: proximityArea

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: thumbArea.left
        anchors.left: parent.left
        enabled: __scrollable
        hoverEnabled: true
        onContainsMouseChanged: {
            if (containsMouse) {
                autohideTimer.restart()
                thumb.shown = true
            }
        }
    }

    MouseArea {
        id: thumbArea

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.left: thumb.left
        enabled: __scrollable
        hoverEnabled: true
        //onClicked: // FIXME: position where the mouse is
    }

    Timer {
        id: autohideTimer

        interval: 1000
        onTriggered: if (!proximityArea.containsMouse && !thumbArea.containsMouse) thumb.shown = false
    }

    Column {
        id: thumb

        anchors.right: slider.right

        property bool shown
        // FIXME: clamp so that thumb never goes beyond the height of the scrollbar
        y: shown ? proximityArea.mouseY - thumb.height / 2 : undefined
        opacity: shown ? 1.0 : 0.0
        Behavior on opacity {NumberAnimation {duration: 100; easing.type: Easing.InOutQuad}}

        MouseArea {
            id: thumbTop

            width: childrenRect.width
            height: childrenRect.height
            onPressed: scrollBy(-targetFlickable.height)
            enabled: __scrollable && thumb.shown

            Image {
                source: parent.pressed ? "artwork/scrollbar_top_pressed.png" : "artwork/scrollbar_top_idle.png"
            }
        }

        MouseArea {
            id: thumbBottom

            width: childrenRect.width
            height: childrenRect.height
            onPressed: scrollBy(targetFlickable.height)
            enabled: __scrollable && thumb.shown

            Image {
                source: parent.pressed ? "artwork/scrollbar_bottom_pressed.png" : "artwork/scrollbar_bottom_idle.png"
            }
        }

        /*
        MouseArea {
            id: dragMouseArea

            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton | Qt.MiddleButton | Qt.RightButton
            drag.target: slider
            drag.axis: Drag.YAxis
            drag.minimumY: 0
            drag.maximumY: scrollbar.height - slider.height

            onPositionChanged: {
                if (drag.active) {
                    targetFlickable.contentY = slider.y * targetFlickable.contentHeight / scrollbar.height
                }
            }
        }

        */
    }

    // Scroll by amount pixels never overshooting
    function scrollBy(amount) {
        var destination = targetFlickable.contentY + amount
        var clampedDestination = Math.max(0, Math.min(targetFlickable.contentHeight - targetFlickable.height,
                                                      destination))
        scrollAnimation.to = clampedDestination
        scrollAnimation.restart()
    }

    SmoothedAnimation {
        id: scrollAnimation

        duration: 200
        easing.type: Easing.InOutQuad
        target: targetFlickable
        property: "contentY"
    }
}
