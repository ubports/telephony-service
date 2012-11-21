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

// define a source property to hold the source URL that was used to create this page
Page {
    property string source
    property int previousTab: -1

    property ListModel chromeButtons
    property bool showChromeBar: true

    signal chromeButtonClicked(var buttonName)
}

/*
The chromeButtons property expects a model formatted like this:

ListModel {
    ListElement {
        label: "A button label"
        name: "buttonName"
    }
}
*/
