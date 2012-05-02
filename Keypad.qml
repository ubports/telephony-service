import QtQuick 1.1

Item {
    width: 188
    height: 62

    Grid {
        rows: 4
        columns: 3

        Repeater {
            model: 9
            delegate: KeypadButton {
                label: modelData + 1
            }
        }

        KeypadButton {
            label: "*"
        }

        KeypadButton {
            label: "0"
        }

        KeypadButton {
            label: "#"
        }
    }
}
