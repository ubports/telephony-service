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

    manager: 'sofiasip'
    protocol: 'sip'
    icon: 'sip'
    hasCrendentials: false
    params: [
        {'name': 'account', 'inputType': 'string', 'label': i18n.tr('Sip Id. (Eg: user@my.sip.net)'), 'store': true},
        {'name': 'password', 'inputType': 'password', 'label': i18n.tr('Password'), 'store': true}
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
