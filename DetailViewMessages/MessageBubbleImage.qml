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

import QtQuick 2.0
import Ubuntu.Components 0.1

// FIXME: when image to small in height, arrow is cropped
Item {
    id: messageBubble

    property alias imageSource: image.source
    property bool mirrored: false
    property int maximumWidth: units.gu(50)
    property int maximumHeight: units.gu(25)

    width: bubble.width
    height: bubble.height

    Item {
        x: bubble.leftPadding - (image.width - image.paintedWidth)/2
        y: bubble.topPadding - (image.height - image.paintedHeight)/2

        Image {
            id: image

            width: messageBubble.maximumWidth - bubble.leftPadding - bubble.rightPadding
            height: messageBubble.maximumHeight - bubble.topPadding - bubble.bottomPadding

            clip: true
            fillMode: Image.PreserveAspectFit
            smooth: true
            cache: false
            asynchronous: true
            source: "../dummydata/fake_mms.jpg"
        }
    }

    BorderImage {
        id: bubble

        property int topPadding: units.gu(0.5)
        property int bottomPadding: units.gu(1)
        property int leftPadding: messageBubble.mirrored ? units.gu(1) : units.gu(2)
        property int rightPadding: messageBubble.mirrored ? units.gu(2) : units.gu(1)

        width: image.paintedWidth + leftPadding + rightPadding
        height: image.paintedHeight + topPadding + bottomPadding
        smooth: true
        source: messageBubble.mirrored ? "../assets/bubble_image_right.png" : "../assets/bubble_image_left.png"
        border {top: units.dp(20); bottom: units.dp(24); left: messageBubble.mirrored ? units.dp(19) : units.dp(27); right: messageBubble.mirrored ? units.dp(27) : units.dp(19)}
    }
}
