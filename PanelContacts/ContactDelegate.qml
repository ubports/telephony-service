import QtQuick 2.0
import Ubuntu.Components.ListItems 0.1 as ListItem

ListItem.Standard {
    width: (parent) ? parent.width : undefined
    __height: 48
    showDivider: true

    // FIXME: this function is used in two places, should be moved to one common place
    text: display
    icon: Qt.resolvedUrl(decoration)
    fallbackIconSource: "../assets/avatar_contacts_list.png"
    __leftIconMargin: 8
    __rightIconMargin: 4
    __iconHeight: 38
    __iconWidth: 38
}
