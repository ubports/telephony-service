/*
 * Copyright 2013 Canonical Ltd.
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
import Ubuntu.Components 0.1 as Theming

Item {
    id: toolbar
    Theming.ItemStyle.class: "toolbar"
    height: visible ? units.gu(6.5) : 0
    anchors.left: parent.left
    anchors.right: parent.right

    signal deleteClicked()
    signal cancelClicked()
    signal saveClicked()

    Component {
        id: toolButtonComponent
        Item {
            id: toolButton
            Theming.ItemStyle.class: "toolbar-button"
            property string text: parent && parent.text ? parent.text : ""
            property url iconSource: parent && parent.iconSource ? parent.iconSource : ""
            signal clicked()
            width: units.gu(5)
            height: toolbar.height

            MouseArea {
                anchors.fill: parent
                onClicked: parent.clicked()
            }
        }
    }

    Loader {
        id: backButton
        sourceComponent: toolButtonComponent

        property string text: tools.back.text
        property string iconSource: tools.back.iconSource

        anchors {
            left: parent.left
            leftMargin: units.gu(2)
            verticalCenter: parent.verticalCenter
        }
        onStatusChanged: {
            if (item && status == Loader.Ready) {
                if (item.hasOwnProperty("clicked")) item.clicked.connect(backButton.itemTriggered);
            }
        }
        signal itemTriggered()
        onItemTriggered: mainView.resetView();
    }

    Row {
        id: buttonsRow
        anchors.right: parent.right
        anchors.rightMargin: units.gu(2)
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        // cancel, save
        Loader {
            id: cancelButton

            property string text: "Cancel"
            property string iconSource: Qt.resolvedUrl("../assets/cancel.png")
            sourceComponent: toolButtonComponent
            anchors.verticalCenter: parent.verticalCenter
            onStatusChanged: {
                if (item && status == Loader.Ready) {
                    if (item.hasOwnProperty("clicked")) item.clicked.connect(cancelButton.itemTriggered);
                }
            }
            signal itemTriggered()
            onItemTriggered: toolbar.cancelClicked()
        }

        Loader {
            id: saveButton

            property string text: "Save"
            property string iconSource: Qt.resolvedUrl("../assets/save.png")
            sourceComponent: toolButtonComponent
            anchors.verticalCenter: parent.verticalCenter
            onStatusChanged: {
                if (item && status == Loader.Ready) {
                    if (item.hasOwnProperty("clicked")) item.clicked.connect(saveButton.itemTriggered);
                }
            }
            signal itemTriggered()
            onItemTriggered: toolbar.saveClicked()
        }
    }
}
