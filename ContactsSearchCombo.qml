// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
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
