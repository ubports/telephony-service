import QtQuick 1.1
import "../Widgets"
import "DetailTypeUtilities.js" as DetailUtils

BaseContactDetailsDelegate {
    id: delegate

    function saveDetail() {
        if (detail && detailTypeInfo.displayField && editor.text.length > 0) {
            console.log("SAVING " + detail + " " + detailTypeInfo.displayField + " > " + editor.text)
            detail[detailTypeInfo.displayField] = (editor.text) ? editor.text : ""
            return true;
        } else return false;
    }

    Item {
        parent: readOnlyContentBox

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: childrenRect.height

        TextCustom {
            id: value

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: subTypeText.left
            anchors.rightMargin: 10
            fontSize: "large"
            elide: Text.ElideRight
            color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
            style: Text.Raised
            styleColor: "white"
            text: (detail && detailTypeInfo.displayField) ? detail[detailTypeInfo.displayField] : ""
        }

        TextCustom {
            id: subTypeText

            anchors.right: parent.right
            anchors.top: parent.top
            horizontalAlignment: Text.AlignRight
            text: DetailUtils.getDetailSubType(detail)
            fontSize: "medium"
            elide: Text.ElideRight
            color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
            style: Text.Raised
            styleColor: "white"
        }
    }

    TextContactDetailsEditor {
        id: editor
        parent: editableContentBox
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        text: value.text
        focus: true
    }
}
