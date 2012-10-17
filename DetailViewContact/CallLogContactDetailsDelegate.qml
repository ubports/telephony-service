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
import "../dateUtils.js" as DateUtils

BaseContactDetailsDelegate {
    id: delegate

    detailTypeInfo: {"actionIcon": "../assets/contact_icon_phone.png"}

    Item {
        parent: readOnlyContentBox

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: childrenRect.height

        TextCustom {
            id: type

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: time.left
            anchors.rightMargin: 10
            fontSize: "small"
            elide: Text.ElideRight
            color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
            style: Text.Raised
            styleColor: "white"
            text: phoneType
        }

        TextCustom {
            id: time

            anchors.top: parent.top
            anchors.right: indicators.left
            anchors.rightMargin: 10
            fontSize: "small"
            elide: Text.ElideRight
            color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
            style: Text.Raised
            styleColor: "white"
            text: DateUtils.formatLogDate(timestamp)
        }

        Row {
            id: indicators

            anchors.right: parent.right
            anchors.rightMargin: 5
            anchors.bottom: time.baseline

            Image {
                source: {
                    if(incoming) {
                        if(missed) {
                            "../assets/missed_call.png"
                        } else {
                            "../assets/received_call.png"
                        }
                    } else {
                        "../assets/outgoing_call.png"
                    }
                }
            }
        }
    }
}
