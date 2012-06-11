import QtQuick 1.1
import "../Widgets"

BaseContactDetailsDelegate {
    id: delegate

    TextCustom {
        id: formattedAddress
        parent: readOnlyContentBox
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        fontSize: "x-large"

        /* Render the address with the same style as in Android */
        function nonEmpty(item) { return item && item.length > 0 }
        text: [
            contactModelItem.street,
            [ [contactModelItem.locality, contactModelItem.region].filter(nonEmpty).join(", "),
              contactModelItem.postcode
            ].filter(nonEmpty).join(" "),
            contactModelItem.country
          ].filter(nonEmpty).join("\n");
    }

    Column {
        parent: editableContentBox
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        TextCustom {
            anchors.left: parent.left
            anchors.right: parent.right
            fontSize: "small"
            text: "Street"
        }

        TextInput {
            id: street
            anchors.left: parent.left
            anchors.right: parent.right
            text: contactModelItem.street
            font.pixelSize: 20
        }

        TextCustom {
            anchors.left: parent.left
            anchors.right: parent.right
            fontSize: "small"
            text: "Locality"
        }

        TextInput {
            id: locality
            anchors.left: parent.left
            anchors.right: parent.right
            text: contactModelItem.locality
            font.pixelSize: 20
        }

        TextCustom {
            anchors.left: parent.left
            anchors.right: parent.right
            fontSize: "small"
            text: "Region"
        }

        TextInput {
            id: region
            anchors.left: parent.left
            anchors.right: parent.right
            text: contactModelItem.region
            font.pixelSize: 20
        }

        TextCustom {
            anchors.left: parent.left
            anchors.right: parent.right
            fontSize: "small"
            text: "Postal Code"
        }

        TextInput {
            id: postcode
            anchors.left: parent.left
            anchors.right: parent.right
            text: contactModelItem.postcode
            font.pixelSize: 20
        }

        TextCustom {
            anchors.left: parent.left
            anchors.right: parent.right
            fontSize: "small"
            text: "Country"
        }

        TextInput {
            id: country
            anchors.left: parent.left
            anchors.right: parent.right
            text: contactModelItem.country
            font.pixelSize: 20
        }
    }
}
