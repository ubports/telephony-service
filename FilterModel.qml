import QtQuick 1.1

QtObject {
    property ListModel proxyModel: null
    property ListModel model: ListModel { }
    property string filter: ""
    property variant fields: null


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
}
