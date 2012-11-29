import QtQuick 2.0
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem

Item {
    id: chromeBar
    property alias buttonsModel: buttonsRepeater.model
    property bool showChromeBar: true
    property bool showBackButton: true
    property variant pageStack

    signal buttonClicked(var buttonName)

    enabled: chromeBar.showChromeBar && ((pageStack && pageStack.depth > 1) || (buttonsRepeater.count > 0))
    anchors.left: parent.left
    anchors.right: parent.right
    height: bar.height - bar.y

    MouseArea {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: bar.height
        drag.target: bar
        drag.axis: Drag.YAxis
        drag.minimumY: 0
        drag.maximumY: height + bar.height
        propagateComposedEvents: true

        property int __pressedY
        onPressed: {
            __pressedY = mouse.y;
        }

        onReleased: {
            // check if there was at least some moving to avoid displaying
            // the chrome bar on clicking
            if (Math.abs(__pressedY - mouse.y) < units.gu(2)) {
                return;
            }

            if (!bar.shown) {
                bar.y = 0;
                bar.shown = true;
            } else {
                bar.y = bar.height;
                bar.shown = false;
            }
        }

        Item {
            id: bar

            property bool shown: false
            height: childrenRect.height
            anchors.left: parent.left
            anchors.right: parent.right
            y: parent.height

            Column {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top

                ListItem.ThinDivider {
                    anchors.left: parent.left
                    anchors.right: parent.right
                }

                Item {
                    id: contents
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: chromeButtons.height + units.gu(2)

                    AbstractButton {
                        id: backButton
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.margins: units.gu(1)
                        width: backImage.width + units.gu(2)
                        height: backImage.height + units.gu(2)
                        visible: pageStack && pageStack.depth > 1
                        onClicked: pageStack.pop()

                        Image {
                            id: backImage
                            anchors.centerIn: parent
                            source: "../assets/back_button.png"
                            fillMode: Image.PreserveAspectFit
                            rotation: 180
                        }
                    }

                    Row {
                        id: chromeButtons
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.margins: units.gu(1)
                        height: childrenRect.height

                        Repeater {
                            id: buttonsRepeater

                            Button {
                                objectName: model.name
                                height: units.gu(4)
                                width: undefined
                                text: model.label
                                onClicked: buttonClicked(model.name)
                            }
                        }
                    }
                }
            }
        }
    }
}
