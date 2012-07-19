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
    id: tabs

    width: 320
    height: 28

    property int currentTab
    property variant model
    property bool buttonsExpanded: true

    /* All TabButtons have the same width.
       A TabButton should never be narrower than __minimumButtonWidth.

       If there is enough space for all the TabButtons to be shown, that is
       if __totalWidthShared is greater than __minimumButtonWidth then:
       - A TabButton should never be wider than the total available width
         shared equally by all the TabButtons.
       - A TabButton should be the width of the widest content of all the
         TabButtons plus twice __buttonPadding.
       If there is not enough space for all the TabButtons to be shown:
       - A TabButton width is __minimumButtonWidth.
    */
    property int __totalWidthShared: tabs.width / repeater.count
    property int __buttonPadding: 12
    property int __minimumButtonWidth: 2*__buttonPadding + 5
    property int __maximumButtonWidth: Math.max(__minimumButtonWidth, __totalWidthShared)
    property bool __needsScrolling: __totalWidthShared < __minimumButtonWidth
    property int __buttonWidestContent
    property int __buttonWidth
    __buttonWidth: {
        if (__needsScrolling) return __minimumButtonWidth
        else if (buttonsExpanded) return __totalWidthShared
        else return Math.min(__maximumButtonWidth, __buttonWidestContent + 2*__buttonPadding)
    }

    function updateButtonWidestContent() {
        var button
        var widestContent = 0
        for (var i=0; i<buttonRow.children.length; i++) {
            button = buttonRow.children[i]
            if (button === repeater) continue
            if (button.contentWidth > widestContent) widestContent = button.contentWidth
        }
        __buttonWidestContent = widestContent
    }
    Component.onCompleted: updateButtonWidestContent()

    Row {
        id: buttonRow

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter

        Repeater {
            id: repeater

            onModelChanged: updateButtonWidestContent()
            onCountChanged: updateButtonWidestContent()
            model: tabs.model
            delegate: TabButton {
                onContentWidthChanged: tabs.updateButtonWidestContent()
                width: tabs.__buttonWidth
                horizontalPadding: __buttonPadding
                count: repeater.count
                isFirst: index == 0
                isLast: index == repeater.count-1
                selected: tabs.currentTab == index
                textColor: Qt.rgba(0.4, 0.4, 0.4, 1.0)
                onClicked: tabs.currentTab = index
                overlayCounter: eval(modelData.eventCountSource)
            }
        }
    }
}
