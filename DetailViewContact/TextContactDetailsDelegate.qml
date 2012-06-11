import QtQuick 1.1
import "../Widgets"

BaseContactDetailsDelegate {
    id: delegate

    property string contactModelProperty

    function save() {
        if (contactModelItem && contactModelProperty) contactModelItem[contactModelProperty] = editor.text
    }

    // Make sure any previous edits are reverted when entering edit mode. Either this or a propert
    // "cancel" implementation is necessary, but this seems simpler.
    onEditableChanged: if (editable) editor.text = value.text

    TextCustom {
        id: value
        parent: readOnlyContentBox
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        fontSize: "x-large"

        text: (contactModelItem && contactModelProperty) ? contactModelItem[contactModelProperty] : ""
    }

    TextInput {
        id: editor
        parent: editableContentBox
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        font.pixelSize: 20

        text: value.text
    }
}
