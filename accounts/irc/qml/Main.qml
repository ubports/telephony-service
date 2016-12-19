import QtQuick 2.0
import Ubuntu.Components 0.1
import Ubuntu.OnlineAccounts.Plugin 1.0

Flickable {
    id: rootFlickable

    property int keyboardSize: Qt.inputMethod.visible ? Qt.inputMethod.keyboardRectangle.height : 0
    contentHeight: editPageLoader.item.height + keyboardSize

    signal finished

    Loader {
        id: editPageLoader
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        sourceComponent: account.accountId != 0 ? existingAccountComponent : newAccountComponent

        Connections {
            target: editPageLoader.item
            onFinished: rootFlickable.finished()
        }
    }

    Component {
        id: newAccountComponent
        NewAccount {}
    }

    Component {
        id: existingAccountComponent
        Options {}
    }
}
