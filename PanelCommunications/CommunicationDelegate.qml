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
import Ubuntu.Components.ListItems 0.1 as ListItem
import "../dateUtils.js" as DateUtils

ListItem.Empty {
    id: communicationDelegate

    function selectIcon()  {
        switch (model.itemType) {
        case "message":
            return "../assets/messages.png";
        case "call":
            if (item.missed) {
                return "../assets/missed-call.png";
            } else if (item.incoming) {
                return "../assets/incoming-call.png";
            } else {
                return "../assets/outgoing-call.png";
            }
        case "group":
            return "../assets/tab_icon_contacts_inactive.png";
        default:
            return "";
        }
    }

    property variant model
    property variant item: (model && model.item) ? model.item : null
    property url avatar: (model && model.contactAvatar) ? model.contactAvatar : ""
    property url itemIcon: selectIcon()
    property variant timestamp: (model && model.timestamp) ? model.timestamp : null
    property string title: (model && model.contactAlias) ? model.contactAlias : ""
    property string subtitle //:"(TODO: show phone type)"
    property string text: (model && model.item && model.item.message) ? model.item.message : ""
    property alias customContentArea: customArea
    property int __spacing: units.gu(1)

    property string __normalFontColor: "#333333"
    property string __selectedFontColor: "#f37505"

    __height: content.height + units.gu(2)

    Rectangle {
        id: selectedOverlay
        anchors.fill: parent
        color: "white"
        opacity: 0.4
        visible: item && item.newItem
    }

    Row {
        id: content
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: __spacing
        anchors.leftMargin: units.gu(2)
        anchors.rightMargin: units.gu(2)
        height: childrenRect.height
        spacing: __spacing

        UbuntuShape {
            height: units.gu(6)
            width: units.gu(6)

            image: Image {
                source: communicationDelegate.avatar != "" ? communicationDelegate.avatar : "../assets/avatar-default.png"
                fillMode: Image.PreserveAspectCrop
                // since we don't know if the image is portrait or landscape without actually reading it,
                // set the sourceSize to be the size we need plus 30% to allow cropping.
                sourceSize.width: width * 1.3
                sourceSize.height: height * 1.3
                asynchronous: true
            }
        }

        Image {
            anchors.top: parent.top
            height: units.gu(2)
            width: units.gu(2)
            source: communicationDelegate.itemIcon
        }

        Column {
            width: parent.width - x

            Label {
                width: parent.width
                visible: text != ""
                elide: Text.ElideRight
                text: communicationDelegate.title
                color: selected ? __selectedFontColor : __normalFontColor
                fontSize: "medium"
                opacity: 0.9
            }

            Label {
                width: parent.width
                visible: text != ""
                elide: Text.ElideRight
                text: DateUtils.formatLogDate(communicationDelegate.timestamp)
                color: selected ? __selectedFontColor : __normalFontColor
                fontSize: "small"
                opacity: 0.6
            }

            Label {
                width: parent.width
                visible: text != ""
                elide: Text.ElideRight
                text: communicationDelegate.subtitle
                color: selected ? __selectedFontColor : __normalFontColor
                font.weight: Font.DemiBold
                fontSize: "small"
                opacity: 0.6
            }

            Label {
                width: parent.width
                visible: text != ""
                text: communicationDelegate.text
                wrapMode: Text.WordWrap
                color: selected ? __selectedFontColor : __normalFontColor
                fontSize: "small"
                opacity: 0.6
            }

            Item {
                id: customArea
                height: childrenRect.height
                width: childrenRect.width
            }
        }
    }
}
