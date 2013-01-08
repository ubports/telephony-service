/*
 * Copyright 2012 Canonical Ltd.
 *
 * This file is part of telephony-app.
 *
 * telephony-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem

Item {
    id: slidingTabsDelegate
    anchors.fill: parent

    clip: true

    property variant item: parent
    property VisualItemModel tabModel: item.__pagesModel

    ListView {
        id: tabView
        objectName: "tabsListView"
        anchors.fill: parent

        model: slidingTabsDelegate.tabModel
        onModelChanged: tabView.updatePages();

        clip: true

        orientation: ListView.Horizontal
        snapMode: ListView.SnapOneItem
        //        snapMode: ListView.SnapToItem
        boundsBehavior: Flickable.DragOverBounds
        highlightFollowsCurrentItem: true

        function updatePages() {
            var tab;
            if (!slidingTabsDelegate.tabModel) return; // not initialized yet

            var tabList = slidingTabsDelegate.tabModel.children
            for (var i=0; i < tabList.length; i++) {
                tab = tabList[i];
                tab.width = tabView.width;
                tab.height = tabView.height;
                tab.anchors.fill = null;
                if (tab.hasOwnProperty("__active")) tab.__active = true;
            }
            tabView.updateSelectedTabIndex();
        }
        onMovingChanged: {
            if(!moving) {
                // update the currentItem
                tabView.currentIndex = contentX / slidingTabsDelegate.width;
                item.selectedTabIndex = tabView.currentIndex;
            }
        }

        function updateSelectedTabIndex() {
            if (tabView.currentIndex === item.selectedTabIndex) return;
            // The view is automatically updated, because highlightFollowsCurrentItem
            tabView.currentIndex = item.selectedTabIndex;
            tabView.positionViewAtIndex(item.selectedTabIndex, ListView.Beginning);
        }

        Connections {
            target: item
            onSelectedTabIndexChanged: tabView.updateSelectedTabIndex();
        }

        Component.onCompleted: {
            tabView.updatePages();
            tabView.positionViewAtIndex(1, ListView.Beginning);
        }

    }

    onWidthChanged: tabView.updatePages();
    onHeightChanged: tabView.updatePages();
    Component.onCompleted: tabView.updatePages();
}
