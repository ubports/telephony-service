import QtQuick 1.1

Item {
    width: 116
    height: 37

    Rectangle {
        anchors.fill: parent
        border.width: 3
        border.color: "#00be4d"
        color: "#3ac974"
        radius: 3
    }

    Image {
        fillMode: Image.PreserveAspectFit
        source: "assets/call_icon.png"
        anchors.centerIn: parent
        width: 23
        smooth: true
    }
}
