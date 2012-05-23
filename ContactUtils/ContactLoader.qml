import QtQuick 1.1
import QtMobility.contacts 1.1

Item {
    id: loader
    property string contactId
    signal contactLoaded(variant contact)

    ContactModel {
        id: contactModel
        manager: "folks"

        filter: DetailFilter {
            detail: ContactDetail.Guid
            field: Guid.guid
            value: loader.contactId != "" ? loader.contactId : "some random string that won't match"
            matchFlags: DetailFilter.MatchFixedString | DetailFilter.MatchCaseSensitive
        }
    }

    // FIXME: this is probably not the correct way to do that, but the ContactModel doesn't have
    // any signal to notify us when the contacts are loaded, so we create this fake delegate that
    // will be created when we get the results from the filtering
    Repeater {
        model: contactModel

        Item {
            id: delegateItem

            Component.onCompleted: {
                // just in case we get more than one contact, check for the index
                if (loader.contactId != "" && index == 0) {
                    loader.contactLoaded(contact);
                }
            }
        }
    }
}
