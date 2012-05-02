import QtQuick 1.1
import ".."

Rectangle {
    id: window

    width: 800
    height: 600

    TextButton {
	width: 200;
	height: 100;
	anchors.centerIn: parent;
        id: testButton1;
        color: "red";
        text: "click me";
        onClicked: { print("clicked"); }
    }  // testButton1
} // window
