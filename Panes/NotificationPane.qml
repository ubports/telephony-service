import QtQuick 2.0
import Ubuntu.Components 0.1

Page {
    property alias image: icon.source

    Item {
        id: background

        anchors.fill: parent

        Image {
            anchors.fill: parent
            source: "../assets/noise_tile.png"
            fillMode: Image.Tile
        }

        Rectangle {
            anchors.fill: parent
            color: "black"
            opacity: 0.05
        }
    }

    Image {
        id: icon

        anchors.centerIn: parent
        anchors.verticalCenterOffset: -units.gu(3)
        fillMode: Image.PreserveAspectFit
    }
}
