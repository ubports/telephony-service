// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import "fontUtils.js" as Font

Rectangle {
    id: favouriteContactList
    width: 320
    height: contactListView.height

    function calculateHeight() {
        var totalHeight = 0
        var i
        for (i=0; i< contacts.count; i=i+1) {
            if(contacts.get(i).favourite)
            totalHeight = totalHeight + 64
        }
        // favourite header size
        totalHeight = totalHeight + Font.sizeToPixels("medium")
        return totalHeight
    }

    ListView {
        id: contactListView
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.left: parent.left
        model: contacts
        height: calculateHeight()
        interactive: false
        header: Item {
            width: favouriteContactList.width
            height: headerText.height
            Text {
                id: headerText
                text: "Favourites"
                font.pixelSize: Font.sizeToPixels("medium")
                font.bold: true
            }
            Rectangle {
                height: 1
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                color: "black"
            }
        }
        delegate: ContactDelegate {
            visible: favourite
            height: visible ? 64 : 0
        }
    }
}
