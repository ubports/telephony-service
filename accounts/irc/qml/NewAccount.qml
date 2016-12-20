import QtQuick 2.0

NewAccountInterface {
    id: root 

    anchors {
        left: parent.left
        right: parent.right
        verticalCenter: parent.verticalCenter
    }

    manager: 'idle'
    protocol: 'irc'
    icon: 'irc'
    hasCrendentials: false
    params: [
        {'name': 'server', 'inputType': 'string', 'label': i18n.tr('Network. (Eg: chat.freenode.net)'), 'store': true},
        {'name': 'account', 'inputType': 'string', 'label': i18n.tr('Nickname'), 'store': true},
        {'name': 'password', 'inputType': 'password', 'label': i18n.tr('Password'), 'store': false}
    ]
    advancedParams: [
        {'name': 'username', 'inputType': 'string', 'label': i18n.tr('Username'), 'store': true},
        {'name': 'fullname', 'inputType': 'string', 'label': i18n.tr('Real name'), 'store': true},
        {'name': 'quit-message', 'inputType': 'string', 'label': i18n.tr('Quit message'), 'store': true},
    ]

    function extendedSettings(inputFields)
    {
        var settings = {}
        settings['Service'] = inputFields['server']
        settings['NickName'] = inputFields['account']
        settings['NormalizedName'] = encodeURI(inputFields['account'])
        //FIXME: maybe move the following params to advancedParams
        settings['param-charset'] = 'UTF-8'
        settings['param-port'] = '6697'
        settings['use-ssl'] = 'true'
        return settings
    }

}
