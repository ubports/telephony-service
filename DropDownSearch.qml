import QtQuick 1.1

Item {
    id: dropDownSearch
    property alias searchQuery: entry.searchQuery
    property alias model: searchView.model
    property alias delegate: searchView.delegate
    property alias listHeight: searchView.height
    property int currentIndex: -1
    state: "idle"

    SearchEntry {
        id: entry
        anchors.fill: parent
        onTextChanged: {
            dropDownSearch.state = "searching"
        }
    }

    ListView {
        id: searchView
        anchors.top: entry.bottom
        anchors.left: entry.left
        anchors.right: entry.right

        MouseArea {
            anchors.fill: parent
            onClicked: {
                currentIndex = searchView.indexAt(mouse.x, mouse.y)
                entry.text =  model.get(currentIndex).displayName
                dropDownSearch.state = "idle"
            }
        }
    }

    states: [
        State {
            name: "searching"
            PropertyChanges { target: searchView; visible: true }
        },
        State {
            name: "idle"
            PropertyChanges { target: searchView; visible: false }
        }
    ]
}
