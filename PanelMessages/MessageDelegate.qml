import QtQuick 1.1
import "../Widgets"
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem

ListItem.Base {
    id: messageDelegate
    height: 60

    CustomListItemBase {
        id: baseInfo
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        iconSource: (avatar != "") ? avatar : placeholderIconSource
        placeholderIconSource: "../assets/avatar_contacts_list.png"
        text: contactAlias
        subtext: message
        textBold: unreadCount > 0
        selected: messageDelegate.selected
    }

    TextCustom {
        id: subsublabel

        anchors.baseline: parent.bottom
        anchors.baselineOffset: -baseInfo.padding + 2
        anchors.right: parent.right
        anchors.rightMargin: baseInfo.padding
        horizontalAlignment: Text.AlignRight
        fontSize: "x-small"

        color: baseInfo.__textColor
        style: Text.Raised
        styleColor: "white"
        opacity: baseInfo.enabled ? 1.0 : 0.5
        // FIXME: show time if today otherwise date without year
        text: Qt.formatDateTime(timestamp, Qt.DefaultLocaleShortDate)
    }
}
