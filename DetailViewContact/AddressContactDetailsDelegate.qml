import QtQuick 1.1
import "../Widgets"

BaseContactDetailsDelegate {
    id: delegate

    function saveDetail() {
        detail.street = editor.street
        detail.locality = editor.locality
        detail.region = editor.region
        detail.postcode = editor.postcode
        detail.country = editor.country
    }

    onEditableChanged: if (editable) {
       editor.street = detail.street
       editor.locality = detail.locality
       editor.region = detail.region
       editor.postcode = detail.postcode
       editor.country = detail.country
    }

    TextCustom {
        id: formattedAddress
        parent: readOnlyContentBox
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        fontSize: "x-large"

        /* Render the address with the same style as in Android */
        function nonEmpty(item) { return item && item.length > 0 }
        text: [
            detail.street,
            [ [detail.locality, detail.region].filter(nonEmpty).join(", "),
              detail.postcode
            ].filter(nonEmpty).join(" "),
            detail.country
          ].filter(nonEmpty).join("\n");
    }

    AddressContactDetailsEditor {
        id: editor
        parent: editableContentBox
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        contactModelItem: delegate.detail
    }
}
