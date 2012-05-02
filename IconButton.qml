import QtQuick 1.1

Rectangle {
    property alias icon: icon.source
    color: "#3a3a3a"
    height: 52
    width: 94

    Image {
        anchors.centerIn: parent
        id: icon
        height: 34
        width: 45
        fillMode: Image.PreserveAspectFit
    }
}

