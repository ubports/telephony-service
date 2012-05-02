import QtQuick 1.1

Item {
    id: abstractButton
    
    property alias color: background.color

    signal clicked

    Rectangle {
	id: background
	radius: 10.0
	width: parent.width
	height: parent.height
    } // background

    MouseArea {
	id: mouseArea
	anchors.fill: parent
	onClicked: {
	    abstractButton.onClicked();
	} // onClicked
    } // mouseArea
} // abstractButton
