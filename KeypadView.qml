import QtQuick 1.1

Item {
    width: keypad.width
    height: childrenRect.height

    anchors.centerIn: parent
    
    KeypadEntry {
        id: keypadEntry
        
        anchors.left: parent.left
        anchors.right: parent.right
    }
    
    Keypad {
        id: keypad
        
        anchors.top: keypadEntry.bottom
        anchors.topMargin: 6
        
        onKeyPressed: keypadEntry.value += label
    }
    
    CallButton {
        anchors.top: keypad.bottom
        anchors.topMargin: 21
        anchors.horizontalCenter: parent.horizontalCenter
    }
}
