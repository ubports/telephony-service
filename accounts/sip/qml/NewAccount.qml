import QtQuick 2.0

NewAccountInterface {
    id: root 


    anchors {
        left: parent.left
        right: parent.right
        verticalCenter: parent.verticalCenter
    }

    manager: 'sofiasip'
    protocol: 'sip'
    icon: 'sip'
    params: [{'name': 'account', 'inputType': 'string', 'label': 'Sip Id', 'store': true},
             {'name': 'password', 'inputType': 'password', 'label': 'Password', 'store': false}]
    advancedParams: []
}
