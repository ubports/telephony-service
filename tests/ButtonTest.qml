import QtQuick 1.1
import ".."

Rectangle {
    id: window

    width: 800
    height: 600

    Column {
        anchors.centerIn: parent
        spacing: 5

        TextButton {
            width: 200
            height: 100
            color: "blue"
            text: "lick me"
            hoverColor: "yellow"
            pressColor: "orange"
            onClicked: { print("clicked (whee)"); }
        }

        IconButton {
            icon: "../assets/call_icon.png"
            verticalMargin: 10
            height: 100
            width: 200
            color: "#008800"
            pressColor: "#004400"
            onClicked: print("hello hello?")
        }
/*
        Button {
            icon: "../assets/call_icon.png"
            verticalMargin: 10
            height: 50
            width: 200
            color: "orange"
            pressColor: "yellow"
            onClicked: print("pressed the coolest button ever!")
            text: "aloha"
        }
*/
    } // column
} // window
