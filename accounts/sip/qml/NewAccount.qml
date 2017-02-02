import QtQuick 2.0

NewAccountInterface {
    id: root 

    manager: 'sofiasip'
    protocol: 'sip'
    icon: 'sip'
    params: [
        {'name': 'account', 'inputType': 'string', 'label': i18n.tr('Sip Id. (Eg: user@my.sip.net)'), 'store': true},
        {'name': 'password', 'inputType': 'password', 'label': i18n.tr('Password'), 'store': false}
    ]
    advancedParams: [
        {'name': 'discover-stun', 'inputType': 'boolean', 'label': i18n.tr('Discover the STUN server automatically'), 'store': true, 'defaultValue' : 'false' },
        {'name': 'stun-server', 'inputType': 'string', 'label': i18n.tr('STUN server'), 'store': true},
        {'name': 'stun-port', 'inputType': 'numeric', 'label': i18n.tr('STUN port'), 'store': true},
        {'name': 'discover-binding', 'inputType': 'boolean', 'label': i18n.tr('Divscover Binding'), 'store': true, 'defaultValue': 'true'},
        {'name': 'proxy-host', 'inputType': 'string', 'label': i18n.tr('Proxy server'), 'store': true},
        {'name': 'port', 'inputType': 'numeric', 'label': i18n.tr('Proxy port'), 'store': true},
        {'name': 'keepalive-mechanism', 'inputType': 'string', 'label': i18n.tr('Keep alive mechanism'), 'store': true, 'defaultValue': 'auto'},
        {'name': 'keepalive-interval', 'inputType': 'numeric', 'label': i18n.tr('Keep alive interval'), 'store': true},
        {'name': 'auth-user', 'inputType': 'string', 'label': i18n.tr('Authentication username'), 'store': true},
        {'name': 'transport', 'inputType': 'string', 'label': i18n.tr('Transport'), 'store': true, 'defaultValue': 'auto'},
        {'name': 'loose-routing', 'inputType': 'boolean', 'label': i18n.tr('Loose Routing'), 'store': true, 'defaultValue': 'false'}
    ]
}
