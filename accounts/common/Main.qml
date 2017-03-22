/*
 * Copyright (C) 2017 Canonical, Ltd.
 *
 * Authors:
 *  Renato Araujo Oliveira Filho <renato.filho@canonical.com>
 *
 * This file is part of telephony-service.
 *
 * telephony-service is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-service is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import Ubuntu.Components 1.3
import Ubuntu.OnlineAccounts.Plugin 1.0

Item {
    id: rootFlickable

    property int keyboardSize: Qt.inputMethod.visible ? Qt.inputMethod.keyboardRectangle.height : 0

    signal finished

    anchors.fill: parent

    Flickable {
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            bottom: btnConfirm.top
        }
        contentWidth: parent.width
        contentHeight: editPageLoader.item.height + keyboardSize
        clip: true

        Loader {
            id: editPageLoader
            sourceComponent: account.accountId != 0 ? existingAccountComponent : newAccountComponent
            anchors {
                left: parent.left
                right: parent.right
            }

            Connections {
                target: editPageLoader.item
                onFinished: rootFlickable.finished()
            }
        }

        Component {
            id: newAccountComponent
            NewAccount {}
        }

        Component {
            id: existingAccountComponent
            Options {}
        }
    }

    Button {
        id: btnConfirm
        text: i18n.tr("Continue")
        color: UbuntuColors.orange
        anchors {
            left: parent.left
            right: parent.right
            bottom: btnCancel.top
            margins: units.gu(2)
            bottomMargin: units.gu(1)
        }
        enabled: editPageLoader.item && editPageLoader.item.isValid
        onClicked: {
            if (editPageLoader.item)
                editPageLoader.item.confirm()
        }

    }

    Button {
        id: btnCancel

        text: i18n.tr("Cancel")
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            margins: units.gu(2)
            bottomMargin: units.gu(1)
        }
        onClicked: {
            if (editPageLoader.item)
                editPageLoader.item.cancel()
        }
    }
}
