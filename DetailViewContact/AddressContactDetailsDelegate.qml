import QtQuick 1.1

BaseContactDetailsDelegate {
    id: delegate

    Text {
        id: formattedAddress
        parent: readOnlyContentBox
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        function nonEmpty(item) { return item && item.length > 0 }
        text: [
            contactModelItem.street,
            [ [contactModelItem.locality, contactModelItem.region].filter(nonEmpty).join(", "),
              contactModelItem.postcode
            ].filter(nonEmpty).join(" "),
            contactModelItem.country
          ].filter(nonEmpty).join("\n");
    }
}
