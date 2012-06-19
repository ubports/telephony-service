import QtQuick 1.1
import "../Widgets"

ListItem {
    id: messageDelegate

    iconSource: avatar == "" ? "../assets/default_avatar.png" : avatar
    text: contactAlias
    subtext: message

    TextCustom {
        id: subsublabel

        anchors.baseline: parent.bottom
        anchors.baselineOffset: -messageDelegate.__padding + 2
        anchors.right: parent.right
        anchors.rightMargin: messageDelegate.__padding
        horizontalAlignment: Text.AlignRight
        fontSize: "small"

        color: messageDelegate.__textColor
        style: Text.Raised
        styleColor: "white"
        opacity: messageDelegate.enabled ? 1.0 : 0.5
        text: Qt.formatDateTime(timestamp, Qt.DefaultLocaleShortDate)
    }
}
