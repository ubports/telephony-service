import QtQuick 1.1
import QtMobility.contacts 1.1

UnionFilter {
    id: filter
    property string filterText: ""

    DetailFilter {
        detail: ContactDetail.DisplayLabel
        field: DisplayLabel.label
        value: filter.filterText
        matchFlags: DetailFilter.MatchContains
    }

    DetailFilter {
        detail: ContactDetail.NickName
        field: Nickname.nickname
        value: filter.filterText
        matchFlags: DetailFilter.MatchContains
    }

    DetailFilter {
        detail: ContactDetail.Presence
        field: Presence.nickname
        value: filter.filterText
        matchFlags: DetailFilter.MatchContains
    }
}

