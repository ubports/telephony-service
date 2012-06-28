import QtQuick 1.1
import "../Widgets"

Item {
    id: name

    property variant detail
    property bool editable

    height: (editable) ? editor.height : text.paintedHeight

    function isNotEmptyString(string) {
        return (string && string.length != 0);
    }

    function formatCustomLabel() {
        // Concatenate all the non empty strings
        return (detail) ?
           [detail.prefix, detail.firstName, detail.middleName, detail.lastName, detail.suffix].filter(isNotEmptyString).join(" ") :
           "";
    }

    function save() {
        detail.firstName = editor.firstName
        detail.middleName = editor.middleName
        detail.lastName = editor.lastName
        detail.prefix = editor.prefix
        detail.suffix = editor.suffix
        detail.customLabel = formatCustomLabel()
    }

    onEditableChanged: if (editable) {
       editor.firstName = detail.firstName
       editor.middleName = detail.middleName
       editor.lastName = detail.lastName
       editor.prefix = detail.prefix
       editor.suffix = detail.suffix
    }

    TextCustom {
        id: text
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.rightMargin: 10
        height: paintedHeight

        fontSize: "x-large"
        elide: Text.ElideRight
        color: Qt.rgba(0.4, 0.4, 0.4, 1.0)
        style: Text.Raised
        styleColor: "white"

        opacity: !editable ? 1.0 : 0.0
        Behavior on opacity {StandardAnimation {}}
        text: (detail && detail.customLabel && detail.customLabel.length > 0) ? detail.customLabel : formatCustomLabel()
    }

    NameContactDetailsEditor {
        id: editor
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.rightMargin: 10
        opacity: editable ? 1.0 : 0.0
        Behavior on opacity {StandardAnimation {}}

        detail: name.detail
    }
}
