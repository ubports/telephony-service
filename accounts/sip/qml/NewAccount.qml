import QtQuick 2.0

NewAccountInterface {
    id: root 

    manager: 'sofiasip'
    protocol: 'sip'
    icon: 'sip'
    params: [{'name': 'account', 'inputType': 'string', 'label': 'Sip Id. (Eg: user@my.sip.net)', 'store': true},
             {'name': 'password', 'inputType': 'password', 'label': 'Password', 'store': false}]
    advancedParams: []
}
