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
import Ubuntu.Components 0.1
import "DetailTypeUtilities.js" as DetailUtils

Item {
    id: chooser

    property variant detail
    property variant detailTypeInfo
    property alias selectedValue: selectedText.subType
    property bool active: false
    enabled: visible
    height: subTypesRow.height

    signal clicked()

    Label {
        id: selectedText

        property var subType: {
            // Use first allowed subtype as default in case there's no detail or no subType
            var subType = DetailUtils.getDetailSubType(detail)
            if (subType.value == "" && detailTypeInfo.subTypes) return detailTypeInfo.subTypes[0]
            else return subType
        }

        anchors.left:  subTypesRow.left
        anchors.verticalCenter: subTypesRow.verticalCenter
        fontSize: "small"
        visible: !active

        text: subType ? subType.label : ""
    }

    MouseArea {
        anchors.fill: parent
        enabled: !active

        onClicked: chooser.clicked()
    }

    Row {
        id: subTypesRow
        height: childrenRect.height
        spacing: units.gu(1)
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        visible: active

        Repeater {
            id: optionsList
            model: (detailTypeInfo.subTypes) ? detailTypeInfo.subTypes : []
            delegate: AbstractButton {
                anchors.top: (parent) ? parent.top : undefined
                height: units.gu(3)
                width: optionText.width

                onClicked: {
                    selectedValue = modelData
                }

                Label {
                    id: optionText

                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    fontSize: "small"
                    color: text == selectedValue.label ? "#dd4814" : Qt.rgba(0.4, 0.4, 0.4, 1.0)
                    text: modelData.label
                }
            }
        }
    }
}
