import QtQuick 2.0
import "../Widgets" as LocalWidgets
import "../dateUtils.js" as DateUtils
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem

ListItem.Base {
    id: messageDelegate
    __height: units.gu(7)
    showDivider: true

    LocalWidgets.CustomListItemBase {
        id: baseInfo

        anchors.fill: parent
        iconSource: (avatar != "") ? avatar : placeholderIconSource
        placeholderIconSource: "../assets/avatar_contacts_list.png"
        text: contactAlias
        subtext: message
        textBold: unreadCount > 0
        selected: messageDelegate.selected

        onClicked: messageDelegate.clicked(mouse)
    }

    TextCustom {
        id: subsublabel

        anchors.baseline: parent.bottom
        anchors.baselineOffset: -baseInfo.padding + units.gu(0.5)
        anchors.right: parent.right
        anchors.rightMargin: baseInfo.padding - units.dp(1)
        horizontalAlignment: Text.AlignRight
        fontSize: "x-small"

        color: baseInfo.__textColor
        style: Text.Raised
        styleColor: "white"
        opacity: messageDelegate.enabled ? 1.0 : 0.5
        text: DateUtils.formatLogDate(timestamp)

    }
}
