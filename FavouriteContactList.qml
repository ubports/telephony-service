// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import "fontUtils.js" as Font

Rectangle {
    id: favouriteContactList
    width: 320
    height: 600

    ListView {
        anchors.fill: parent
        model: contacts
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
