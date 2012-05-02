import QtQuick 1.1

Item {
    property alias searchQuery: entry.searchQuery
    property alias model: searchView.model
    property alias delegate: searchView.delegate
    property alias listHeight: searchView.height

    SearchEntry {
        id: entry
        anchors.fill: parent
    }

    ListView {
        id: searchView
        anchors.top: entry.bottom
        anchors.left: entry.left
        anchors.right: entry.right
    }
}
