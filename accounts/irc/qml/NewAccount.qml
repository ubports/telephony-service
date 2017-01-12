import QtQuick 2.0

NewAccountInterface {
    id: root

    anchors {
        left: parent.left
        right: parent.right
        verticalCenter: parent.verticalCenter
    }

    manager: 'irc'
    protocol: 'irc'
    icon: 'irc'
    hasCrendentials: false
    params: [
        {'name': 'server', 'inputType': 'string', 'label': i18n.tr('Network. (Eg: chat.freenode.net)'), 'store': true},
        {'name': 'nickname', 'inputType': 'string', 'label': i18n.tr('Nickname'), 'store': true},
    ]
    advancedParams: [
        {'name': 'port', 'inputType': 'numeric', 'label': i18n.tr('Port'), 'store': true},
        {'name': 'use-ssl', 'inputType': 'boolean', 'label': i18n.tr('Use ssl'), 'store': true},
        {'name': 'verify-ssl-cert', 'inputType': 'boolean', 'label': i18n.tr('Verify ssl certificate'), 'store': true},
        {'name': 'username', 'inputType': 'string', 'label': i18n.tr('Username'), 'store': true},
        {'name': 'password', 'inputType': 'password', 'label': i18n.tr('Password'), 'store': true},
        {'name': 'fullname', 'inputType': 'string', 'label': i18n.tr('Real name'), 'store': true},
    ]

    function extendedSettings(inputFields)
    {
        var settings = {}
        settings['param-account'] = inputFields['nickname'] + "@" + inputFields['server']
        if (settings['param-port'] == "")
            settings['param-port'] = "6667"
        return settings
    }
}
