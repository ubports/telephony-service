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

        // FIXME: QtMobility contacts plugin is not propagating the filterChanged signal in
        // UnionFilter and IntersectionFilter, so the proper solution is to get the fix in there
        onFilterChanged: filter.filterChanged()
    }

    DetailFilter {
        detail: ContactDetail.NickName
        field: Nickname.nickname
        value: filter.filterText
        matchFlags: DetailFilter.MatchContains

        // FIXME: this should be fixed in mobility, see above
        onFilterChanged: filter.filterChanged()
    }

    DetailFilter {
        detail: ContactDetail.Presence
        field: Presence.nickname
        value: filter.filterText
        matchFlags: DetailFilter.MatchContains

        // FIXME: this should be fixed in mobility, see above
        onFilterChanged: filter.filterChanged()
    }
}

