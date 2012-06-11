import QtQuick 1.1

BaseContactDetailsDelegate {
    id: delegate

    property string contactModelProperty

    Text {
        parent: readOnlyContentBox
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        text: (contactModelItem && contactModelProperty) ? contactModelItem[contactModelProperty] : ""
    }
}
