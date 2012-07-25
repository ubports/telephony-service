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
    id: scrollbar

    property real pageSize
    property real contentPosition
    property real contentSize

    property real __visibleSizeRatio: Math.min(pageSize / contentSize, 1.0)
    property real __visiblePosition: contentPosition / contentSize
    property bool __scrollable: __visibleSizeRatio != 1.0

    width: 30

    opacity: __scrollable ? 1.0 : 0.0
    Behavior on opacity {NumberAnimation {duration: 100; easing.type: Easing.InOutQuad}}

    function __clamp(x, min, max) {
        return Math.max(min, Math.min(max, x))
    }

    /* Scroll by amount pixels never overshooting */
    function __scrollBy(amount) {
        var destination = contentPosition + amount
        scrollAnimation.to = __clamp(destination, 0, contentSize - pageSize)
        scrollAnimation.restart()
    }

    function __scrollOnePageUp() {
        __scrollBy(-pageSize)
    }

    function __scrollOnePageDown() {
        __scrollBy(pageSize)
    }

    SmoothedAnimation {
        id: scrollAnimation

        duration: 200
        easing.type: Easing.InOutQuad
        target: scrollbar
        property: "contentPosition"
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

        height: __clamp(__visibleSizeRatio * scrollbar.height, minimalHeight, scrollbar.height)
        Behavior on height {NumberAnimation {duration: 200; easing.type: Easing.InOutQuad}}

        Binding {
            target: slider
            property: "y"
            value: {
                var yPosition = __clamp(__visiblePosition, 0, 1-__visibleSizeRatio)
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

    MouseArea {
        id: thumbArea

        property int thumbMiddleY: thumb.y + thumb.height / 2
        property int thumbBottomY: thumb.y + thumb.height
        property bool isInThumbTop: mouseY > thumb.y && mouseY < thumbMiddleY
        property bool isInThumbBottom: mouseY > thumbMiddleY && mouseY < thumbBottomY

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.left: thumb.left
        /* Wider target area more tolerant to mistakes */
        anchors.leftMargin: -8
        enabled: __scrollable
        hoverEnabled: true
        onEntered: thumb.show()
        onPressed: {
            if (mouseY < thumb.y) {
                thumb.placeThumbTopUnderMouse(mouse)
            } else if (mouseY > thumbBottomY) {
                thumb.placeThumbBottomUnderMouse(mouse)
            }
        }
        onClicked: {
            if (isInThumbTop) {
                __scrollOnePageUp()
            } else if (isInThumbBottom) {
                __scrollOnePageDown()
            }
        }

        /* Dragging behaviour */
        property int dragYStart
        property int dragYAmount: thumbArea.drag.target.y - thumbArea.dragYStart
        property int thumbYStart
        property int contentYStart
        drag {
            target: Item {}
            /* necessary to make sure drag is activated even by a non vertical movement */
            axis: Drag.XandYAxis
            onActiveChanged: if (drag.active) {dragYStart = drag.target.y; thumbYStart = thumb.y; contentYStart = contentPosition}
        }

        /* The content scrolls differently depending on where the thumb is
           relative to the slider:
            - if the thumb is 'connected' to the slider, the thumb allows the
              user to scroll the entire content, from top to bottom
            - if the thumb is not 'connected' to the slider, the thumb is fixed
              to the slider and allows for precision scrolling of a small part
              of the content
        */
        Binding {
            target: scrollbar
            property: "contentPosition"
            value: {
                if (contentSize <= thumbArea.height * 2 || thumb.isDetachedFromSlider) {
                    // precision scrolling: the thumb is fixed to the slider
                    // FIXME: when clamped, reset dragging
                    return __clamp(thumbArea.contentYStart + thumbArea.dragYAmount, 0, contentSize - pageSize)
                } else {
                    // proportional scrolling: all the content is reachable
                    return thumb.y / (scrollbar.height - thumb.height) * (contentSize - pageSize)
                }
            }
            when: thumbArea.drag.active
        }

        Binding {
            target: thumb
            property: "y"
            value: __clamp(thumbArea.thumbYStart + thumbArea.dragYAmount, thumb.minimumY, thumb.maximumY)
            when: thumbArea.drag.active
        }
    }

    Timer {
        id: autohideTimer

        interval: 1000
        repeat: true
        onTriggered: if (!proximityArea.containsMouse && !thumbArea.containsMouse && !thumbArea.pressed) thumb.hide()
    }

    Item {
        id: thumb

        anchors.right: slider.right
        width: childrenRect.width
        height: childrenRect.height

        property bool shown
        property int minimumY: 0
        property int maximumY: scrollbar.height - thumb.height
        property bool isDetachedFromSlider: thumb.y + thumb.height <= slider.y || thumb.y >= slider.y + slider.height

        /* Show the thumb as close as possible to the mouse pointer */
        onShownChanged: {
            if (shown) {
                var mouseY = proximityArea.containsMouse ? proximityArea.mouseY : thumbArea.mouseY
                y = __clamp(mouseY - thumb.height / 2, thumb.minimumY, thumb.maximumY)
            }
        }

        function show() {
            autohideTimer.restart()
            shown = true
        }

        function hide() {
            autohideTimer.stop()
            shown = false
        }

        function placeThumbTopUnderMouse(mouse) {
            yAnimation.to = __clamp(mouse.y - height / 4, minimumY, maximumY)
            yAnimation.restart()
        }

        function placeThumbBottomUnderMouse(mouse) {
            yAnimation.to = __clamp(mouse.y - height * 3 / 4, minimumY, maximumY)
            yAnimation.restart()
        }

        NumberAnimation {
            id: yAnimation

            duration: 100
            easing.type: Easing.InOutQuad
            target: thumb
            property: "y"
        }

        opacity: shown ? (thumbArea.containsMouse || thumbArea.drag.active ? 1.0 : 0.5) : 0.0
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
