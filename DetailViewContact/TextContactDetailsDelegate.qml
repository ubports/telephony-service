import QtQuick 1.1
import "../Widgets"

BaseContactDetailsDelegate {
    id: delegate

    function saveDetail() {
        if (detail && detailTypeInfo.displayField) {
            console.log("SAVING " + detail + " " + detailTypeInfo.displayField + " > " + editor.text)
            detail[detailTypeInfo.displayField] = (editor.text) ? editor.text : ""
        }
    }

    TextCustom {
        id: value

        parent: readOnlyContentBox
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        fontSize: "large"
        elide: Text.ElideRight
        color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
        style: Text.Raised
        styleColor: "white"
        text: (detail && detailTypeInfo.displayField) ? detail[detailTypeInfo.displayField] : ""
    }

    TextContactDetailsEditor {
        id: editor
        parent: editableContentBox
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        text: value.text
    }
}
