import QtQuick 2.0

import Ubuntu.Components 0.1
import Ubuntu.OnlineAccounts 0.1

Flickable {
    id: rootFlickable

    signal finished

    clip: true
    anchors {
        left: parent.left
        right: parent.right
        top: parent.top
    }
    height: units.gu(20)
    contentHeight: contents.height

    Loader {
        id: contents

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }

        source: Qt.resolvedUrl("./NewAccount.qml")

        Connections {
            target: contents.item
            onFinished: rootFlickable.finished()
        }
    }
}
