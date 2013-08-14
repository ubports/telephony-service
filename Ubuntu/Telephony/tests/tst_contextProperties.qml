/*
 * Copyright (C) 2013 Canonical, Ltd.
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
import QtTest 1.0
import Ubuntu.Telephony 0.1

TestCase {
    id: contextPropertiesTest
    name: "ContextPropertiesTest"

    function test_applicationUtils() {
        verify(applicationUtils != undefined, "applicationUtils is not defined");
    }

    function test_telepathyHelper() {
        verify(telepathyHelper != undefined, "telepathyHelper is not defined");
    }

    function test_chatManager() {
        verify(chatManager != undefined, "chatManager is not defined");
    }

    function test_callManager() {
        verify(callManager != undefined, "callManager is not defined");
    }
}
