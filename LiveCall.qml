import QtQuick 1.0
import "fontUtils.js" as Font

Item {

    Column {
        anchors.fill: parent

        Image {
            id: picture
            anchors.horizontalCenter: parent.horizontalCenter

            source: "assets/picture_anna.png"
            height: sourceSize.height
            width: sourceSize.width
        }

        Text {
            id: name
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Anna Olson"
            font.pixelSize: Font.sizeToPixels("xx-large")
        }

        Text {
            id: number
            anchors.horizontalCenter: parent.horizontalCenter
            text: "+23 453-245-2321"
            font.pixelSize: Font.sizeToPixels("x-large")
        }

        Text {
            id: location
            anchors.horizontalCenter: parent.horizontalCenter
            text: "London"
            font.pixelSize: Font.sizeToPixels("x-large")
        }

        StopWatch {
            time: "00:54"
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
}
