import QtQuick 1.1
import "../Widgets"

BaseContactDetailsDelegate {
    id: delegate

    property string contactModelProperty

    function save() {
        if (detail && contactModelProperty) detail[contactModelProperty] = editor.text
    }

    onEditableChanged: editor.text = value.text

    TextCustom {
        id: value
        parent: readOnlyContentBox
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        fontSize: "x-large"

        text: (detail && contactModelProperty) ? detail[contactModelProperty] : ""
    }

    TextContactDetailsEditor {
        id: editor
        parent: editableContentBox
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
    }
}
