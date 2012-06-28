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
  FIXME: document behaviours

  Example usage:

  Flickable {
      id: flickable
  }

  Scrollbar {
      targetFlickable: flickable
  }
*/

// FIXME:
// - make scrolling by dragging not 1-to-1 but instead ensuring that the whole area can be browsed
// - try to make the thumb follow the mouse when in sliderArea maybe using a timer to ease the targetting of parts of the thumb
Item {
    id: scrollbar

    property variant targetFlickable
    property bool __scrollable: targetFlickable.visibleArea.heightRatio != 1.0

    width: 30
    anchors.right: targetFlickable.right
    anchors.top: targetFlickable.top
    anchors.bottom: targetFlickable.bottom

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
        }
    }

    Rectangle {
        id: sliderThumbConnector

        anchors.right: parent.right
        width: slider.width

        property int thumbMargin: 3
        property bool isThumbAboveSlider: thumb.y < slider.y

        y: isThumbAboveSlider ? thumb.y + thumbMargin : slider.y + slider.height
        height: isThumbAboveSlider ? (slider.y) - y : (thumb.y + thumb.height - thumbMargin) - y

        color: "white"
        opacity: thumb.shown ? 1.0 : 0.0
        Behavior on opacity {NumberAnimation {duration: 100; easing.type: Easing.InOutQuad}}
    }

    function clamp(x, min, max) {
        return Math.max(min, Math.min(max, x))
    }

    MouseArea {
        id: proximityArea

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: sliderArea.left
        anchors.left: parent.left
        enabled: __scrollable
        hoverEnabled: true
        onEntered: thumb.show()
    }

    Binding {
        target: thumb
        property: "y"
        value: clamp(proximityArea.mouseY - thumb.height / 2, thumb.minimumY, thumb.maximumY)
        when: proximityArea.containsMouse && thumb.shown
    }

    MouseArea {
        id: sliderArea

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.left: thumb.left
        enabled: __scrollable
        hoverEnabled: true
        onEntered: thumb.show()
        onClicked: {
            var targetY
            var goingUp = sliderArea.mouseY < thumb.y
            if (goingUp) {
                scrollOnePageUp()
                targetY = sliderArea.mouseY - thumb.height / 4
            } else {
                targetY = sliderArea.mouseY - thumb.height * 3 / 4
            }
            thumb.y = clamp(targetY, thumb.minimumY, thumb.maximumY)
        }
    }

    Timer {
        id: autohideTimer

        interval: 1000
        onTriggered: if (!proximityArea.containsMouse && !sliderArea.containsMouse) thumb.shown = false
    }

    Item {
        id: thumb

        anchors.right: slider.right
        width: thumbVisual.width
        height: thumbVisual.height

        property bool shown
        property int minimumY: 0
        property int maximumY: scrollbar.height - thumb.height

        function show() {
            autohideTimer.restart()
            thumb.shown = true
        }

        opacity: shown ? 1.0 : 0.0
        Behavior on opacity {NumberAnimation {duration: 100; easing.type: Easing.InOutQuad}}

        MouseArea {
            id: thumbArea

            property bool isInThumbTop: mouseY < thumb.height / 2

            anchors.fill: parent
            onClicked: if (isInThumbTop) scrollOnePageUp(); else scrollOnePageDown()
            enabled: __scrollable && thumb.shown

            // dragging behaviour
            property int lastDragY
            onPressed: lastDragY = mouse.y
            drag {
                target: thumb
                axis: Drag.YAxis
                minimumY: thumb.minimumY
                maximumY: thumb.maximumY
            }
            onMouseYChanged: if (drag.active) targetFlickable.contentY = clamp(targetFlickable.contentY + mouseY - lastDragY, 0, targetFlickable.contentHeight - targetFlickable.height)
        }

        Column {
            id: thumbVisual

            Image {
                id: thumbTop
                source: thumbArea.isInThumbTop && thumbArea.pressed ? "artwork/scrollbar_top_pressed.png" : "artwork/scrollbar_top_idle.png"
            }
            Image {
                id: thumbBottom
                source: !thumbArea.isInThumbTop && thumbArea.pressed ? "artwork/scrollbar_bottom_pressed.png" : "artwork/scrollbar_bottom_idle.png"
            }
        }
    }

    // Scroll by amount pixels never overshooting
    function scrollBy(amount) {
        var destination = targetFlickable.contentY + amount
        var clampedDestination = Math.max(0, Math.min(targetFlickable.contentHeight - targetFlickable.height,
                                                      destination))
        scrollAnimation.to = clampedDestination
        scrollAnimation.restart()
    }

    function scrollOnePageUp() {
        scrollBy(-targetFlickable.height)
    }

    function scrollOnePageDown() {
        scrollBy(targetFlickable.height)
    }

    SmoothedAnimation {
        id: scrollAnimation

        duration: 200
        easing.type: Easing.InOutQuad
        target: targetFlickable
        property: "contentY"
    }
}
