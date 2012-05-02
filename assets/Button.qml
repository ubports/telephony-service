import QtQuick 1.1

Item {
    id: button
    
    property alias color: background.color
    property alias text: label.text

    //width: 200; height: 50 // should we have a default button size?

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
	    button.onClicked();
	} // onClicked
    } // mouseArea

    Text {
	id: label
	anchors.centerIn: parent
	font.pointSize: 24;
    } // label

    //Component.onCompleted: mouseArea.onClicked.connect(clicked) // doesn't work in QtQuick1?

} // button
