import QtQuick 2.0
import Ubuntu.Components 0.1

Item {
    id: header
    property string text
    height: childrenRect.height
    clip: true

    anchors {
        top: parent.top
        left: parent.left
        right: parent.right
    }

    Column {
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        height: childrenRect.height

        Rectangle {
            id: topDivider
            height: units.dp(2)
            anchors {
                left: parent.left
                right: parent.right
            }
            color: "#f37505"
        }

        Item {
            id: headerLabel
            height: units.gu(6)
            anchors {
                left: parent.left
                right: parent.right
            }

            Label {
                text: header.text
                anchors {
                    verticalCenter: parent.verticalCenter
                    left: parent.left
                    leftMargin: units.gu(2)
                }
                fontSize: "x-large"
                color: "#333333"
                opacity: 0.4
                height: units.gu(4)
                verticalAlignment: Text.AlignVCenter
            }
        }

        Image {
            id: divider
            anchors {
                left: parent.left
                right: parent.right
            }
            height: units.gu(2)
            // FIXME: replace by the correct asset when it is done
            source: "../assets/section_divider.png"
        }
    }
}
