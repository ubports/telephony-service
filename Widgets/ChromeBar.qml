import QtQuick 2.0
import Ubuntu.Components 0.1

Item {
    anchors.left: parent.left
    anchors.right: parent.right
    height: visible ? childrenRect.height + units.gu(2) : 0
    visible: telephony.view.showChromeBar && (telephony.viewStack.depth > 1) || (buttonsRepeater.count > 0)

    onVisibleChanged: console.log("Visible changed to " + visible)

    AbstractButton {
            id: backButton
            anchors {
                left: parent.left
                top: parent.top
                bottom: parent.bottom
                margins: units.gu(1)
            }
            width: backImage.width + units.gu(2)
            height: backImage.height + units.gu(2)
            visible: telephony.viewStack.depth > 1
            onClicked: telephony.viewStack.pop()

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
            model: telephony.view.chromeButtons ? telephony.view.chromeButtons : null

            Button {
                height: units.gu(4)
                text: model.label
                onClicked: telephony.view.chromeButtonClicked(model.name)
            }
        }
    }
}
