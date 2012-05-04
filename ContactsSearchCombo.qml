import QtQuick 1.1

DropDownSearch {
    id: search
    model: contacts
    delegate: SearchContactDelegate {
        filter: search.searchQuery
    }
    listHeight: 300
    clip: false
}
