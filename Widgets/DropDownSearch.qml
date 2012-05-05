import QtQuick 1.1

// FIXME: BUG WHEN SEARCHING

Item {
    id: dropDownSearch
    property alias searchQuery: entry.searchQuery
    property alias model: searchView.model
    property alias delegate: searchView.delegate
    property int listMaximumHeight: 400
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
    height: entry.height

    SearchEntry {
        id: entry
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 30
        onTextChanged: text != "" ? dropDownSearch.state = "searching" : dropDownSearch.state = "idle"
        onLeftIconClicked: parent.leftIconClicked()
        onRightIconClicked: parent.rightIconClicked()
    }

    ListView {
        id: searchView
        anchors.top: entry.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        // FIXME: Does not uses contentHeight here
        height: visible ? (contentHeight >= listMaximumHeight ? listMaximumHeight : contentHeight) : 0

        MouseArea {
            anchors.fill: parent
            onClicked: {
                currentIndex = searchView.indexAt(mouse.x, mouse.y)
                var currentItem = model.get(currentIndex)
                if (currentItem) {la
                    entry.text =  currentItem.dispyName
                    dropDownSearch.state = "idle"
                    itemSelected(currentItem)
                }
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
