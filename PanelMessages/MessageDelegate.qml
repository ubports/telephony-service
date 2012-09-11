import QtQuick 1.1
import "../Widgets"
import Ubuntu.Components 0.1

ListItem {
    id: messageDelegate

    iconSource: (avatar != "") ? avatar : placeholderIconSource
    placeholderIconSource: "../assets/avatar_contacts_list.png"
    text: contactAlias
    subtext: message
    textBold: unreadCount > 0
    height: 60

    TextCustom {
        id: subsublabel

        anchors.baseline: parent.bottom
        anchors.baselineOffset: -messageDelegate.__padding + 2
        anchors.right: parent.right
        anchors.rightMargin: messageDelegate.__padding
        horizontalAlignment: Text.AlignRight
        fontSize: "x-small"

        color: messageDelegate.__textColor
        style: Text.Raised
        styleColor: "white"
        opacity: messageDelegate.enabled ? 1.0 : 0.5
        // FIXME: show time if today otherwise date without year
        text: Qt.formatDateTime(timestamp, Qt.DefaultLocaleShortDate)
    }
}
