import QtQuick 1.1

Item {
    id: keypad

    width: childrenRect.width
    height: childrenRect.height

    signal keyPressed(int keycode, string label)

    Grid {
        rows: 4
        columns: 3

        Repeater {
            model: [Qt.Key_1, Qt.Key_2, Qt.Key_3, Qt.Key_4, Qt.Key_5, Qt.Key_6, Qt.Key_7, Qt.Key_8, Qt.Key_9]
            delegate: KeypadButton {
                label: index + 1
                keycode: modelData
                onClicked: keypad.keyPressed(keycode, label)
            }
        }

        KeypadButton {
            label: "*"
            keycode: Qt.Key_Asterisk
            onClicked: keypad.keyPressed(keycode, label)
        }

        KeypadButton {
            label: "0"
            keycode: Qt.Key_0
            onClicked: keypad.keyPressed(keycode, label)
        }

        KeypadButton {
            label: "#"
            keycode: Qt.Key_ssharp
            onClicked: keypad.keyPressed(keycode, label)
        }
    }
}
