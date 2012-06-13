import QtQuick 1.1
import "../Widgets"

ListItem {
    // FIXME: reference to fake model needs to be removed before final release
    property variant contact: fakeContacts.fromId(contactId)

    // FIXME: the dummydata prefix should be in the model
    iconSource: "../dummydata/" + contact.photo
    text: contact.displayName
    subtext: lastMessageContent
    sussubtext: lastMessageDate
}
