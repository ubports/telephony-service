import QtQuick 2.0
import Ubuntu.Components 1.3

Loader {
    id: root

    property var model
    readonly property string value: status === Loader.Ready ? item.value : ""
    readonly property bool isEmpty: status === Loader.Ready ? item.isEmpty : true

    Component {
        id: stringField

        TextField {
            id: field

            property alias label: field.placeholderText
            property string defaultValue

            readonly property alias value: field.text
            readonly property bool isEmpty: value === "" || (defaultValue && value === model.defaultValue)

            text: defaultValue ? defaultValue : ""
        }
    }

    Component {
        id: booleanField

        Item {
            property alias label: fieldLabel.text
            property string defaultValue

            readonly property string value: fieldValue.checked ? "true" : "false"
            readonly property bool isEmpty: (defaultValue && value === model.defaultValue)

            height: fieldValue.height

            Label {
                id: fieldLabel

                anchors {
                    left: parent.left
                    right: fieldValue.left
                    verticalCenter: parent.verticalCenter
                }
            }
            Switch {
                id: fieldValue

                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                }
                checked: (defaultValue && (defaultValue === 'true'))
            }
        }
    }

    Component {
       id: numericField

       TextField {
           id: field

           property alias label: field.placeholderText
           property string defaultValue

           readonly property alias value: field.text
           readonly property bool isEmpty: value === "" || (defaultValue && (value === defaultValue))

           inputMethodHints: Qt.ImhDigitsOnly
           validator: IntValidator {}
       }
    }

    Component{
        id: passwordField

        Item {
            property alias label: field.placeholderText
            readonly property alias value: field.text
            readonly property bool isEmpty: value === ""

            height: field.height + showPasswordCheck.height
            TextField {
                id: field

                echoMode: showPasswordCheck.checked ? TextInput.Normal : TextInput.Password
                anchors {
                    left: parent.left
                    right: parent.right
                }
            }
            CheckBox {
                id: showPasswordCheck
                anchors {
                    left: field.left
                    top: field.bottom
                    topMargin: units.gu(1)
                }
            }
            Label {
                text: i18n.tr("Show Password")
                anchors {
                    top: showPasswordCheck.top
                    left: showPasswordCheck.right
                    leftMargin: units.gu(1)
                    right: field.right
                }
            }
        }
    }


    sourceComponent: {
        if (!model)
            return null

        if (!model.inputType) {
            console.warn("Model does not contain 'inputType'")
            return null
        }

        switch (model.inputType) {
        case 'string':
            return stringField
        case 'boolean':
            return booleanField
        case 'numeric':
            return numericField
        case 'password':
            return passwordField
        }
    }

    Binding {
        target: root.item
        property: "label"
        value: model.label
        when: root.status == Loader.Ready
    }

    Binding {
        target: root.item
        property: "defaultValue"
        value: model.defaultValue
        when: model.hasOwnProperty('defaultValue') && root.status == Loader.Ready
    }
}
