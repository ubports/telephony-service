import QtQuick 1.1

Rectangle {
    width: 800
    height: 600

    Item {
        id: leftPane
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        Tabs {
            id: tabs
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
        }

        width: 250
 
        Loader {
            id: leftPaneContent
            anchors.top: tabs.bottom
            anchors.bottom: leftPane.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            source: {
                switch (tabs.currentTab) {
                case 0:
                    ""
                    break;
                case 1:
                    ""
                    break;
                case 2:
                    "ContactsPanel.qml"
                    break;
                }
            }
        }
    }

    Item {
        id: rightPane
        anchors.left: leftPane.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        Rectangle {
            anchors.fill: parent
            color: "#ebebeb"
        }
    }
}
