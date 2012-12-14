import QtQuick 2.0

Item {
    id: keyboardRect
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    height: Qt.inputMethod.visible ? Qt.inputMethod.keyboardRectangle.height : 0

    Behavior on height {
        StandardAnimation { }
    }

    function recursiveFindFocusedItem(parent) {
        if (parent.activeFocus) {
            return parent;
        }

        for (var i in parent.children) {
            var child = parent.children[i];
            if (child.activeFocus) {
                return child;
            }

            var item = recursiveFindFocusedItem(child);

            if (item != null) {
                return item;
            }
        }

        return null;
    }

    Connections {
        target: Qt.inputMethod

        onVisibleChanged: {
            if (!Qt.inputMethod.visible) {
                var focusedItem = recursiveFindFocusedItem(keyboardRect.parent);
                if (focusedItem != null) {
                    focusedItem.focus = false;
                }
            }
        }
    }
}
