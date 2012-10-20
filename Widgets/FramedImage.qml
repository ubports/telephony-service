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

Item {
    id: framedImage

    property url source
    property url fallbackSource
    property bool frameVisible: true
    property bool darkBorder: false
    property url frameSource

    Image {
        id: image

        anchors.fill: parent
        smooth: true
        asynchronous: true
        fillMode: Image.PreserveAspectFit
        sourceSize.width: width

        function isSourceDefined(sourceUrl) {
            return sourceUrl != "" && sourceUrl != undefined
        }

        function tryLoadingSource() {
            if (isSourceDefined(framedImage.source)) {
                image.source = framedImage.source
            } else if (isSourceDefined(framedImage.fallbackSource)) {
                image.source = framedImage.fallbackSource
            }
        }

        Connections {
            target: framedImage
            onSourceChanged: image.tryLoadingSource()
            onFallbackSourceChanged: image.tryLoadingSource()
        }
        Component.onCompleted: tryLoadingSource()

        onStatusChanged: {
            if (status == Image.Error && isSourceDefined(framedImage.fallbackSource)) {
                image.source = framedImage.fallbackSource
            }
        }
    }

    BorderImage {
        id: frame

        property bool big: framedImage.width >= units.dp(100)
        property int borderSize: big ? 6 : 3

        source: {
            if (framedImage.frameSource != "") {
                return framedImage.frameSource
            }

            if (big) {
                return "artwork/ListItemFrameBigDark.png"
            } else {
                return framedImage.darkBorder ? "artwork/ListItemFrameDark.png" : "artwork/ListItemFrame.png"
            }
        }

        anchors.fill: image
        anchors.topMargin: big ? -units.dp(1) : 0
        anchors.bottomMargin: big ? -units.dp(2) : -units.dp(1)
        anchors.leftMargin: big ? -units.dp(1) : 0
        anchors.rightMargin: big ? -units.dp(1) : 0

        border {
            left: borderSize
            right: borderSize
            top: borderSize
            bottom: borderSize
        }
        horizontalTileMode: BorderImage.Stretch
        verticalTileMode: BorderImage.Stretch
        visible: framedImage.frameVisible
    }
}
