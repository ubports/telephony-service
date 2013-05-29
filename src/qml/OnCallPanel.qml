/*
 * Copyright 2012-2013 Canonical Ltd.
 *
 * This file is part of phone-app.
 *
 * phone-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * phone-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import Ubuntu.PhoneApp 0.1
import "Widgets" as LocalWidgets
import Ubuntu.Components 0.1

AbstractButton {
    id: onCallPanel

    property QtObject call: callManager.foregroundCall

    height: units.gu(8)

    BorderImage {
        id: background

        anchors.fill: parent
        source: "assets/oncall_background.png"
        horizontalTileMode: BorderImage.Repeat
        verticalTileMode: BorderImage.Repeat
        border {top: units.dp(5); bottom: 0; left: 0; right: 0}
    }


    Item {
        id: avatar

        anchors.top: parent.top
        anchors.topMargin: units.gu(2)
        anchors.left: parent.left
        anchors.leftMargin: units.gu(1)
        width: units.gu(5)
        height: units.gu(5)

        LocalWidgets.FramedImage {
            id: avatarIcon

            anchors.fill: parent
            anchors.margins: units.dp(1)
            source: call ? call.contactAvatar : fallbackSource
            fallbackSource: "assets/avatar-default.png"
            frameSource: "assets/oncall_picture_frame.sci"
        }

        Image {
            id: avatarEmblem

            source: "assets/oncall_phone_icon.png"
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.topMargin: -units.gu(0.5)
            anchors.leftMargin: -units.dp(1)
        }
    }

    Column {
        anchors.left: avatar.right
        anchors.leftMargin: units.gu(1)
        anchors.right: parent.right
        anchors.rightMargin: units.gu(0.5)
        anchors.verticalCenter: avatar.verticalCenter

        Label {
            anchors.left: parent.left
            anchors.right: parent.right

            text: i18n.tr("On Call")
            fontSize: "small"
            elide: Text.ElideRight
            color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
            style: Text.Raised
            styleColor: "white"
        }

        Label {
            anchors.left: parent.left
            anchors.right: parent.right

            text: {
                if (call) {
                    if (call.voicemail) {
                        i18n.tr("Voicemail")
                    } else {
                        call.phoneNumber
                    }
                } else {
                    ""
                }
            }
            fontSize: "large"
            elide: Text.ElideRight
            color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
            style: Text.Raised
            styleColor: "white"
        }
    }

    Item {
        id: duration

        visible: stopWatch.time > 0
        anchors.top: parent.top
        anchors.topMargin: units.gu(2)
        anchors.right: parent.right
        anchors.rightMargin: units.gu(1)
        height: units.gu(3)
        width: stopWatch.width

        BorderImage {
            id: durationBackground

            source: "assets/oncall_call_duration_frame.png"
            anchors.fill: parent
            horizontalTileMode: BorderImage.Stretch
            verticalTileMode: BorderImage.Stretch
            border {left: units.dp(10); right: units.dp(10); top: units.dp(12); bottom: units.dp(12)}
        }

        LocalWidgets.StopWatch {
            id: stopWatch

            anchors.centerIn: parent
            fontSize: "small"
            color: "#5d960f"
            time: call ? call.elapsedTime : 0
        }
    }
}