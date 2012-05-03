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

            Connections {
                target: leftPaneContent.item
                onCallContactRequested: {
                    // To keep this simple we rely on the fact that setting source to a
                    // local file will immadiately make the item availalable.
                    rightPaneContent.source = "LiveCall.qml"
                    rightPaneContent.item.startCall(contact)
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

            Loader {
                id: rightPaneContent
                anchors.fill: parent

                Connections {
                    target: rightPaneContent.item
                    ignoreUnknownSignals: true
                    onCallEnded: rightPaneContent.source = ""
                }
            }
        }
    }
}
