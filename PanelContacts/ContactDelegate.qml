import QtQuick 2.0
import Ubuntu.Components.ListItems 0.1 as ListItem

ListItem.Standard {
    width: (parent) ? parent.width : undefined
    __height: units.gu(6)
    showDivider: true

    // FIXME: this function is used in two places, should be moved to one common place
    text: display
    icon: Qt.resolvedUrl(decoration)
    fallbackIconSource: "../assets/avatar_contacts_list.png"
    __leftIconMargin: units.gu(1)
    __rightIconMargin: units.gu(0.5)
    __iconHeight: units.gu(5)
    __iconWidth: units.gu(5)
}
