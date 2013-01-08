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

QtObject {
    property ListModel proxyModel: null
    property ListModel model: ListModel { }
    property string filter: ""
    property variant fields: null
    property int proxyCount: proxyModel ? proxyModel.count : 0

    function checkFilter(element) {
        if (filter.length == 0) {
            return true
        }

        var lowerFilter = filter.toLowerCase()
        for (var i=0; i < fields.length; i++) {
            var value = element[fields[i]].toLowerCase()
            if (value.indexOf(lowerFilter) >= 0)
                return true
        }

        return false
    }

    function applyFilter() {
        model.clear();
        for(var i=0; i < proxyModel.count; i++) {
            var element = proxyModel.get(i)
            if (checkFilter(element)) {
                model.append(element)
            }
        }
    }

    onProxyModelChanged: { applyFilter() }
    onFilterChanged: { applyFilter() }
    onProxyCountChanged: applyFilter()
}
