import QtQuick 2.0
import "../Widgets" as LocalWidgets
import "../dateUtils.js" as DateUtils
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem

Item {
    id: messageDelegate

    Component.onCompleted: {
        // mark the message as read
        item.newItem = false;
    }

    LocalWidgets.CustomListItemBase {
        id: baseInfo

        anchors.fill: parent
        iconSource: (contactAvatar != "") ? contactAvatar : placeholderIconSource
        placeholderIconSource: "../assets/avatar_contacts_list.png"
        text: contactAlias
        subtext: item.message
        //textBold: unreadCount > 0

        onClicked: messageDelegate.parent.clicked(mouse)
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
