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

// define a source property to hold the source URL that was used to create this page
Page {
    id: page
    property string source
    property int previousTab: -1
    property variant headerContents: null

    onActiveChanged: updateHeader()
    onHeaderChanged: updateHeader()
    onHeaderContentsChanged: updateHeader()

    function updateHeader() {
        if (page.header && page.headerContents != null) {
            if (active) {
                page.header.ItemStyle.delegate = page.headerContents;
                page.header.height = page.headerContents.height;
            } else {
                page.headerContents.parent = page;
                page.header.ItemStyle.delegate = null;
            }
        }
    }
}
