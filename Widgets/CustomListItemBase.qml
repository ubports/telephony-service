/*
 * Copyright 2012 Canonical Ltd.
 *
 * This file is part of telephony-app.
 *
 * telephony-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import Ubuntu.Components 0.1

AbstractButton {
    id: listItem

    width: units.gu(31)

    property bool isIcon: false
    property alias iconSource: icon.source
    property url placeholderIconSource
    property alias text: label.text
    property alias subtext: sublabel.text
    property bool selected: false
    property bool textBold: false
    property int padding: units.gu(1)
    property bool incoming: true
    property alias content: contentContainer

    property bool __hasSubLabel: subtext != ""
    property color __textColor: (selected || textBold) ? "#f37505" : Qt.rgba(0.4, 0.4, 0.4, 1.0)

    Item {
        id: body
        anchors.fill: parent

        Item {
            id: iconContainer

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: incoming ? parent.left : undefined
            anchors.right: !incoming ? parent.right : undefined
            width: units.gu(7)

            FramedImage {
                id: icon

                anchors.centerIn: parent
                anchors.verticalCenterOffset: units.dp(1)
                width: listItem.isIcon ? units.gu(2) : units.gu(5)
                height: width
                fallbackSource: listItem.placeholderIconSource
                frameVisible: !listItem.isIcon
                opacity: listItem.enabled ? 1.0 : 0.5
            }
        }

        Item {
            id: contentContainer
            anchors.right: incoming ? parent.right : iconContainer.left
            anchors.left: incoming ? iconContainer.right : parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            Label {
                id: label

                property bool centered: !listItem.__hasSubLabel

                anchors.top: !centered ? parent.top : undefined
                anchors.topMargin: !centered ? listItem.padding - units.gu(0.5) : 0
                anchors.left: parent.left
                anchors.leftMargin: units.dp(1)
                anchors.right: parent.right
                anchors.rightMargin: listItem.padding
                anchors.verticalCenter: centered ? parent.verticalCenter : undefined
                fontSize: "medium"
                font.bold: textBold
                horizontalAlignment: !incoming ? Text.AlignRight : Text.AlignLeft
                elide: Text.ElideRight

                color: listItem.__textColor
                style: Text.Raised
                styleColor: "white"
                opacity: listItem.enabled ? 1.0 : 0.5
            }

            Label {
                id: sublabel

                anchors.left: label.left
                anchors.leftMargin: label.anchors.leftMargin
                anchors.top: label.bottom
                anchors.topMargin: units.dp(1)
                anchors.right: parent.right
                anchors.rightMargin: listItem.padding
                fontSize: "small"
                font.bold: textBold
                elide: Text.ElideRight
                horizontalAlignment: !incoming ? Text.AlignRight : Text.AlignLeft

                color: listItem.__textColor
                style: Text.Raised
                styleColor: "white"
                opacity: listItem.enabled ? 1.0 : 0.5
            }
        }
    }
}
