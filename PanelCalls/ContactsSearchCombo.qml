import QtQuick 1.1
import "../Widgets"

DropDownSearch {
    id: search
    model: contacts
    delegate: SearchContactDelegate {
        filter: search.searchQuery
    }
    listHeight: 300
    clip: false
}
