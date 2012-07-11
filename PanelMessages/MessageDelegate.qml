import QtQuick 1.1
import QtMobility.contacts 1.1
import "../Widgets"
import "../"

ListItem {
    id: messageDelegate
    property variant contact: contactWatcher.contact

    Binding { target: contactWatcher; property: "number"; value: phoneNumber; }
    Binding { target: contactWatcher; property: "contactId"; value: customId; }

    ContactWatcher {
        id: contactWatcher
    }

    iconSource: contact && contact.avatar != "" ? contact.avatar : ""
    placeholderIconSource: "../assets/avatar_contacts_list.png"
    text: contact ? contact.displayLabel : contactAlias
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
        // FIXME: show time if today otherwise date without year
        text: Qt.formatDateTime(timestamp, Qt.DefaultLocaleShortDate)
    }
}
