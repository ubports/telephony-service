/*
 * Copyright (C) 2017 Canonical, Ltd.
 *
 * Authors:
 *  Renato Araujo Oliveira Filho <renato.filho@canonical.com>
 *
 * This file is part of telephony-service.
 *
 * telephony-service is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-service is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

    function formatDisplayName(inputFields)
    {
        return inputFields['nickname'] + "@" + inputFields['server']
    }
}
