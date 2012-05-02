import QtQuick 1.1

AbstractButton {
    id: textButton
    
    property alias text: label.text

    Text {
	id: label
	anchors.centerIn: parent
	font.pointSize: 24;
    } // label

} // button
