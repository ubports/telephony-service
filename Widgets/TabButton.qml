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

AbstractButton {
    id: tabButton

    property int count
    property alias iconSource: icon.source
    property alias text: label.text
    property alias textSize: label.fontSize
    property alias textColor: label.color
    property bool selected
    property bool isFirst
    property bool isLast
    property int contentWidth: text != "" ? invisibleLabel.paintedWidth : icon.paintedWidth
    property int horizontalPadding

    anchors.top: parent.top
    anchors.bottom: parent.bottom

    Component.onCompleted: {
        var objectString =  "import TelephonyApp 0.1;";
        objectString += "import QtQuick 1.1;";
        objectString += "import \".\";";
        objectString += "TabCountOverlay {";
        objectString += "   id: overlay;";
        objectString += "   count: " + modelData.eventCountSource + ";";
        objectString += "   selected: parent.selected;";
        objectString += "}";

        var counter = Qt.createQmlObject(objectString, tabButton, "foo");
        counter.anchors.verticalCenter = icon.top
        counter.anchors.horizontalCenter = icon.left
    }

    BorderImage {
        id: background

        anchors.fill: parent
        source: {
            if (isFirst) {
                return selected ? "artwork/TabLeftSelected.png" : "artwork/TabLeftUnselected.png"
            } else if (isLast) {
                return selected ? "artwork/TabRightSelected.png" : "artwork/TabRightUnselected.png"
            } else {
                return selected ? "artwork/TabMiddleSelected.png" : "artwork/TabMiddleUnselected.png"
            }
        }

        border { left: isFirst ? 9 : 1; top: isFirst || isLast ? 9 : 2; right: isLast ? 10 : 2; bottom: 0 } // FIXME: take into account isFirst, isLast
        horizontalTileMode: BorderImage.Stretch
        verticalTileMode: BorderImage.Stretch
    }

    Image {
        id: icon

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.topMargin: 6
        anchors.bottomMargin: 4

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: isFirst ? 2 : 0
        fillMode: Image.PreserveAspectFit
        source: selected ? modelData.iconSelected : modelData.iconUnselected
    }

    TextCustom {
        id: label

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: tabButton.horizontalPadding
        horizontalAlignment: Text.AlignHCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -1
        fontSize: "large"
        text: modelData.text ? modelData.text : ""
        elide: Text.ElideRight
    }

    /* Invisible label that is used for width computations */
    TextCustom {
        id: invisibleLabel

        visible: false
        text: label.text
        fontSize: label.fontSize
    }
}
