import QtQuick 1.1
import "../Widgets"

ListItem {
    width: (parent) ? parent.width : undefined

    // FIXME: this function is used in two places, should be moved to one common place
    text: display
    iconSource: decoration ? decoration : placeholderIconSource
    placeholderIconSource: "../assets/avatar_contacts_list.png"
}
