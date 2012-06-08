import QtQuick 1.1
import QtMobility.contacts 1.1
import "../Widgets"

Column {
    id: contactDetailsSection
    visible: details.count > 0
    property string name
    property alias model: details.model
    property alias delegate: details.delegate

    Item {
        anchors.left: parent.left
        anchors.right: parent.right
        height: section.paintedHeight + 10
        TextCustom {
            id: section
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            text: name
        }
    }

    Repeater {
        id: details
        // model and delegate are set via property alias
    }
}
