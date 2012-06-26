import QtQuick 1.1
import "../Widgets"

Column {
    id: editor

    property variant contactModelItem
    property alias street: street.value
    property alias locality: locality.value
    property alias region: region.value
    property alias postcode: postcode.value
    property alias country: country.value

    spacing: 1

    AddressEditorField {
        id: street

        anchors.left: parent.left
        anchors.right: parent.right
        value: contactModelItem.street
        description: "Street"
    }

    AddressEditorField {
        id: locality

        anchors.left: parent.left
        anchors.right: parent.right
        value: contactModelItem.locality
        description: "Locality"
    }

    AddressEditorField {
        id: region

        anchors.left: parent.left
        anchors.right: parent.right
        value: contactModelItem.region
        description: "Region"
    }

    AddressEditorField {
        id: postcode

        anchors.left: parent.left
        anchors.right: parent.right
        value: contactModelItem.postcode
        description: "Postal Code"
    }

    AddressEditorField {
        id: country

        anchors.left: parent.left
        anchors.right: parent.right
        value: contactModelItem.country
        description: "Country"
    }
}
