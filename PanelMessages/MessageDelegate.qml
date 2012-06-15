import QtQuick 1.1
import "../Widgets"

ListItem {
    id: messageDelegate

    // FIXME: reference to fake model needs to be removed before final release
    property variant contact: fakeContacts.fromId(contactId)

    // FIXME: the dummydata prefix should be in the model
    iconSource: "../dummydata/" + contact.photo
    text: contact.displayName
    subtext: lastMessageContent

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
        text: lastMessageDate
    }
}
