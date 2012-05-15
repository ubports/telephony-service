import QtQuick 1.1
import "../Widgets"

// FIXME: port to use the QtMobility contacts model
// FIXME: filter using the model filtering, not using the delegate trick
DropDownSearch {
    id: search
    model: contacts
    delegate: SearchContactDelegate {
        filter: search.searchQuery
    }
    listMaximumHeight: 300
}
