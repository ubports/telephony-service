import QtQuick 1.1
import "../Widgets"
import Ubuntu.Components.ListItems 0.1 as ListItem

ListItem.Standard {
    width: (parent) ? parent.width : undefined

    // FIXME: this function is used in two places, should be moved to one common place
    text: display
    iconSource: decoration
    fallbackIconSource: "../assets/avatar_contacts_list.png"
    __leftIconMargin: 8
    __rightIconMargin: 4
}
