import QtQuick 1.1

BaseContactDetailsDelegate {
    id: delegate

    property string contactModelProperty

    Text {
        id: valueText
        parent: readOnlyContentBox
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        text: (contactModelItem && contactModelProperty) ? contactModelItem[contactModelProperty] : ""
    }

    TextInput {
        parent: editableContentBox
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        text: valueText.text
        //onTextChanged: contactDetailsItem.fieldValueChanged(text)
    }
}
