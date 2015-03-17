/*
 * Copyright (C) 2014 Canonical, Ltd.
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
import Ubuntu.Telephony.PhoneNumber 0.1
import "tst_PhoneNumberData.js" as TestData

TestCase {
    id: phoneNumberFieldTest
    name: "PhoneNumberFieldTest"

    function init()
    {
        TestData.reset_item(phoneField)
    }

    function test_updateOnlyWhenFocused()
    {
        phoneField.updateOnlyWhenFocused = false
        compare(phoneField.updateOnlyWhenFocused, false)
    }

    function test_formatPhone_data()
    {
        return TestData.formatPhone_data()
    }

    function test_formatPhone(data)
    {
        phoneField.insert(0, data.input)
        tryCompare(phoneField, "text", data.expectedOutput)
        tryCompare(phoneField, "cursorPosition", data.expectedCursorPosition)
    }

    function test_modifyPhone_data()
    {
        return TestData.modifyPhone_data()
    }

    function test_modifyPhone(data)
    {
        phoneField.text = data.input
        tryCompare(phoneField, "text", data.formatedInput)

        phoneField.cursorPosition = data.moveCursor
        tryCompare(phoneField, "cursorPosition", data.moveCursor)

        switch (data.action) {
        case "remove":
            phoneField.remove(data.moveCursor, data.moveCursor-1)
            break;
        case "insert":
            phoneField.insert(data.moveCursor, data.text)
        }
        tryCompare(phoneField, "text", data.newFormatedInput)
        tryCompare(phoneField, "cursorPosition", data.expectedCursorPosition)
    }

    function test_update_autoFormater()
    {
        phoneField.autoFormat = false
        phoneField.text = "7572923"
        compare(phoneField.text, "7572923")
        phoneField.autoFormat = true
        compare(phoneField.text, "757-2923")
    }

    PhoneNumberField {
        id: phoneField

        focus: true
        autoFormat: true
        defaultRegion: "US"
    }
}
