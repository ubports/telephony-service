import QtQuick 1.1
import "../Widgets"

BaseContactDetailsDelegate {
    id: delegate

    function save() {
        console.log("SAVING " + detail + " " + detailTypeInfo.displayField + " > " + editor.text)
        if (detail && detailTypeInfo.displayField)
            detail[detailTypeInfo.displayField] = (editor.text) ? editor.text : ""
    }

    onEditableChanged: editor.text = value.text

    TextCustom {
        id: value
        parent: readOnlyContentBox
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        fontSize: "x-large"

        text: (detail && detailTypeInfo.displayField) ? detail[detailTypeInfo.displayField] : ""
    }

    TextContactDetailsEditor {
        id: editor
        parent: editableContentBox
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
    }
}
