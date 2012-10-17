import QtQuick 2.0
import TelephonyApp 0.1
import Ubuntu.Components 0.1
import "../Widgets" as LocalWidgets

LocalWidgets.TelephonyPage {
    title: "Call Log"
    Item {
        id: background

        anchors.fill: parent

        Image {
            anchors.fill: parent
            source: "../assets/noise_tile.png"
            fillMode: Image.Tile
        }

        Rectangle {
            anchors.fill: parent
            color: "black"
            opacity: 0.05
        }
    }

    Tabs {
        id: buttonGroup

        anchors.top: parent.top
        anchors.topMargin: 7
        anchors.left: parent.left
        anchors.right: parent.right

        separator: Rectangle {
            height: 1
            color: "white"
            opacity: 0.65
        }

        Tab {
            title: "All"
        }

        Tab {
            id: missedCallsTab
            title: "Missed"
        }
    }

    CallLogProxyModel {
        id: proxyModel
        logModel: callLogModel
        onlyMissedCalls: buttonGroup.selectedTabIndex == 1
    }

    Item {
        id: logBackground

        anchors.top: buttonGroup.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        Image {
            anchors.fill: parent
            source: "../assets/right_pane_pattern.png"
            fillMode: Image.Tile
        }

        Rectangle {
            anchors.fill: parent
            color: "white"
            opacity: 0.3
        }
    }

    ListView {
        id: callLogView

        clip: true
        anchors.fill: logBackground
        // FIXME: references to runtime and fake model need to be removed before final release
        model: typeof(runtime) != "undefined" ? fakeCallLog : proxyModel

        delegate: CallLogDelegate {
            width: parent.width
            onContactClicked: telephony.showContactDetailsFromId(contactId)
            onActionClicked: telephony.callNumber(phoneNumber)
        }
    }

    LocalWidgets.ScrollbarForListView {
        view: callLogView
    }
}
