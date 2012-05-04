import QtQuick 1.0
import "../Widgets"

Rectangle {
    property int time: 0
    radius: 4
    color: "#3a3a3a"
    height: count.paintedHeight + 8 * 2
    width: count.paintedWidth + 22 * 2

    function pad(text, length) {
        while (text.length < length) text = '0' + text;
        return text;
    }

    TextCustom {
        id: count
        anchors.centerIn: parent
        color: "white"
        text: {
            var divisor_for_minutes = time % (60 * 60);
            var minutes = String(Math.floor(divisor_for_minutes / 60));

            var divisor_for_seconds = divisor_for_minutes % 60;
            var seconds = String(Math.ceil(divisor_for_seconds));

            return "%1:%2".arg(pad(minutes, 2)).arg(pad(seconds, 2));
        }
        fontSize: "x-large"
    }
}
