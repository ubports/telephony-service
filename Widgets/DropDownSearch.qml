import QtQuick 1.1

Item {
    id: dropDownSearch
    property alias searchQuery: entry.searchQuery
    property alias model: searchView.model
    property alias delegate: searchView.delegate
    property alias listHeight: searchView.height
    property alias text: entry.text
    property int currentIndex: -1

    property alias leftIconSource: entry.leftIconSource
    property alias leftIconVisible: entry.leftIconVisible
    property alias rightIconSource: entry.rightIconSource
    property alias rightIconVisible: entry.rightIconVisible

    signal leftIconClicked
    signal rightIconClicked

    signal itemSelected(variant item)
    state: "idle"

    SearchEntry {
        id: entry
        anchors.fill: parent
        onTextChanged: dropDownSearch.state = "searching"
        onLeftIconClicked: parent.leftIconClicked()
        onRightIconClicked: parent.rightIconClicked()
    }

    ListView {
        id: searchView
        anchors.top: entry.bottom
        anchors.left: entry.left
        anchors.right: entry.right
        height: visible ? (contentHeight > 400 ? 400 : contentHeight) : 0

        MouseArea {
            anchors.fill: parent
            onClicked: {
                currentIndex = searchView.indexAt(mouse.x, mouse.y)
                var currentItem = model.get(currentIndex)
                entry.text =  currentItem.displayName
                dropDownSearch.state = "idle"
                itemSelected(currentItem)
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
