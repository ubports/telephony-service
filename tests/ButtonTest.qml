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
            onClicked: { print("clicked (whee)"); }
        }

        TextButton {
            width: 200
            height: 100
            color: "red"
            text: "Don't click me"
            onClicked: print("don't do that again!")
            borderColor: "black"
            borderWidth: 2
            radius: 10
        }

        IconButton {
            icon: "../assets/call_icon.png"
            verticalMargin: 10
            height: 100
            width: 200
            color: "green"
            onClicked: print("hello hello?")
        }
    } // column
} // window
