import QtQuick 1.1
import "../fontUtils.js" as Font
import "../"

Rectangle {
    id: favouriteContactList

    property alias filter: filterModel.filter

    width: 320
    height: contactListView.height
    signal contactClicked(variant contact)

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

    FilterModel {
        id: filterModel
        proxyModel: contacts
        fields: ["displayName", "phone"]
    }

    ListView {
        id: contactListView
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.left: parent.left
        model: filterModel.model
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
            onClicked: favouriteContactList.contactClicked(contacts.get(index))
        }
    }
}
