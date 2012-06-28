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
      targetFlickable: flickable
  }
*/
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

    function __clamp(x, min, max) {
        return Math.max(min, Math.min(max, x))
    }

    /* Scroll by amount pixels never overshooting */
    function __scrollBy(amount) {
        var destination = targetFlickable.contentY + amount
        scrollAnimation.to = __clamp(destination, 0, targetFlickable.contentHeight - targetFlickable.height)
        scrollAnimation.restart()
    }

    function __scrollOnePageUp() {
        __scrollBy(-targetFlickable.height)
    }

    function __scrollOnePageDown() {
        __scrollBy(targetFlickable.height)
    }

    SmoothedAnimation {
        id: scrollAnimation

        duration: 200
        easing.type: Easing.InOutQuad
        target: targetFlickable
        property: "contentY"
    }

    /* The slider's position represents which part of the flickable is visible.
       The slider's size represents the size the visible part relative to the
       total size of the flickable.
    */
    Rectangle {
        id: slider

        property int minimalHeight: 40

        anchors.right: parent.right
        width: 2
        color: "#fc7134"

        height: __clamp(targetFlickable.visibleArea.heightRatio * scrollbar.height, minimalHeight, scrollbar.height)
        Behavior on height {NumberAnimation {duration: 200; easing.type: Easing.InOutQuad}}

        Binding {
            target: slider
            property: "y"
            value: {
                var yPosition = __clamp(targetFlickable.visibleArea.yPosition, 0, 1-targetFlickable.visibleArea.heightRatio)
                return yPosition * scrollbar.height
            }
        }
    }

    /* The sliderThumbConnector ensures a visual connection between the slider and the thumb */
    Rectangle {
        id: sliderThumbConnector

        property bool isThumbAboveSlider: thumb.y < slider.y

        anchors.left: slider.left
        anchors.right: slider.right
        anchors.top: isThumbAboveSlider ? thumb.top : slider.bottom
        anchors.bottom: isThumbAboveSlider ? slider.top : thumb.bottom
        anchors.topMargin: isThumbAboveSlider ? 3 : 0
        anchors.bottomMargin: isThumbAboveSlider ? 0 : 3

        color: "white"
        opacity: thumb.shown ? 1.0 : 0.0
        Behavior on opacity {NumberAnimation {duration: 100; easing.type: Easing.InOutQuad}}
    }

    /* The thumb appears whenever the mouse gets close enough to the scrollbar
       and disappears after being for a long enough time far enough of it
    */
    MouseArea {
        // FIXME: clicking on the proximityArea eats clicks
        id: proximityArea

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: thumbArea.left
        enabled: __scrollable
        hoverEnabled: true
        onEntered: thumb.show()
    }

    /* While the mouse is in the proximityArea, the thumb follows it */
    Binding {
        target: thumb
        property: "y"
        value: __clamp(proximityArea.mouseY - thumb.height / 2, thumb.minimumY, thumb.maximumY)
        when: proximityArea.containsMouse && thumb.shown
    }

    MouseArea {
        id: thumbArea

        property bool isInThumbTop: mouseY < thumb.y + thumb.height / 2

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.left: thumb.left
        enabled: __scrollable
        hoverEnabled: true
        onEntered: thumb.show()
        onPressed: {
            if (mouseY < thumb.y) {
                thumb.placeThumbTopUnderMouse(mouse)
            } else if (mouseY > thumb.y + thumb.height) {
                thumb.placeThumbBottomUnderMouse(mouse)
            }
        }
        onClicked: {
            if (isInThumbTop) {
                __scrollOnePageUp()
            } else {
                __scrollOnePageDown()
            }
        }

        // dragging behaviour
        drag {
            target: thumb
            axis: Drag.YAxis
            minimumY: thumb.minimumY
            maximumY: thumb.maximumY
        }

        Binding {
            target: targetFlickable
            property: "contentY"
            value: thumb.y / (scrollbar.height - thumb.height) * (targetFlickable.contentHeight - targetFlickable.height)
            when: thumbArea.drag.active
        }
    }

    Timer {
        id: autohideTimer

        interval: 1000
        onTriggered: if (!proximityArea.containsMouse && !thumbArea.containsMouse) thumb.shown = false
    }

    Item {
        id: thumb

        anchors.right: slider.right
        width: childrenRect.width
        height: childrenRect.height

        property bool shown
        property int minimumY: 0
        property int maximumY: scrollbar.height - thumb.height

        function show() {
            autohideTimer.restart()
            thumb.shown = true
        }

        function placeThumbTopUnderMouse(mouse) {
            y = __clamp(mouse.y - height / 4, minimumY, maximumY)
        }

        function placeThumbBottomUnderMouse(mouse) {
            y = __clamp(mouse.y - height * 3 / 4, minimumY, maximumY)
        }

        opacity: shown ? 1.0 : 0.0
        Behavior on opacity {NumberAnimation {duration: 100; easing.type: Easing.InOutQuad}}

        Column {
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
}
