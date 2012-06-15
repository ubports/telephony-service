import QtQuick 1.1
import "../Widgets"

Column {
    id: editor
    property variant contactModelItem
    property alias street: street.text
    property alias locality: locality.text
    property alias region: region.text
    property alias postcode: postcode.text
    property alias country: country.text

    TextCustom {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 4
        anchors.leftMargin: 4
        fontSize: "small"
        text: "Street"
    }

    Rectangle {
        border.width: 1
        border.color: "black"
        anchors.left: parent.left
        anchors.right: parent.right
        height: childrenRect.height + 4

        TextInput {
            id: street
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 4
            anchors.leftMargin: 4
            text: contactModelItem.street
            font.pixelSize: 20
        }
    }

    TextCustom {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 4
        anchors.leftMargin: 4
        fontSize: "small"
        text: "Locality"
    }

    Rectangle {
        border.width: 1
        border.color: "black"
        anchors.left: parent.left
        anchors.right: parent.right
        height: childrenRect.height + 4

        TextInput {
            id: locality
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 4
            anchors.leftMargin: 4
            text: contactModelItem.locality
            font.pixelSize: 20
        }
    }

    TextCustom {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 4
        anchors.leftMargin: 4
        fontSize: "small"
        text: "Region"
    }

    Rectangle {
        border.width: 1
        border.color: "black"
        anchors.left: parent.left
        anchors.right: parent.right
        height: childrenRect.height + 4

        TextInput {
            id: region
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 4
            anchors.leftMargin: 4
            text: contactModelItem.region
            font.pixelSize: 20
        }
    }

    TextCustom {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 4
        anchors.leftMargin: 4
        fontSize: "small"
        text: "Postal Code"
    }

    Rectangle {
        border.width: 1
        border.color: "black"
        anchors.left: parent.left
        anchors.right: parent.right
        height: childrenRect.height + 4

        TextInput {
            id: postcode
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 4
            anchors.leftMargin: 4
            text: contactModelItem.postcode
            font.pixelSize: 20
        }
    }

    TextCustom {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 4
        anchors.leftMargin: 4
        fontSize: "small"
        text: "Country"
    }

    Rectangle {
        border.width: 1
        border.color: "black"
        anchors.left: parent.left
        anchors.right: parent.right
        height: childrenRect.height + 4

        TextInput {
            id: country
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 4
            anchors.leftMargin: 4
            text: contactModelItem.country
            font.pixelSize: 20
        }
    }
}
