import QtQuick 1.1
import "../Widgets"

ListItem {
    width: (parent) ? parent.width : undefined

    // FIXME: this function is used in two places, should be moved to one common place
    function contactName() {
        if (!contact)
            return "";
        if (contact.displayLabel != "")
            return contact.displayLabel
        else if (contact.nickname.nickname != "")
            return contact.nickname.nickname;
        else if (contact.presence.nickname != "")
            return contact.presence.nickname;
    }

    text: contactName()
    iconSource: (contact && contact.avatar.imageUrl && (contact.avatar.imageUrl.toString().length > 0))
                ? contact.avatar.imageUrl : "../assets/default_avatar.png"
}
