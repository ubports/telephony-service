import QtQuick 2.0
import Ubuntu.Components.ListItems 0.1 as ListItem

ListItem.Standard {
    width: (parent) ? parent.width : undefined
    __height: units.dp(48)
    showDivider: true

    // FIXME: this function is used in two places, should be moved to one common place
    text: display
    icon: Qt.resolvedUrl(decoration)
    fallbackIconSource: "../assets/avatar_contacts_list.png"
    __leftIconMargin: units.dp(8)
    __rightIconMargin: units.dp(4)
    __iconHeight: units.dp(38)
    __iconWidth: units.dp(38)
}
