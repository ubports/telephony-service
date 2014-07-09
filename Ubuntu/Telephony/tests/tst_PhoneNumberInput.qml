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
    id: phoneNumberInputTest
    name: "PhoneNumberInputTest"

    function init()
    {
        TestData.reset_item(phoneInput)
    }

    function test_updateOnlyWhenFocused()
    {
        phoneInput.updateOnlyWhenFocused = false
        compare(phoneInput.updateOnlyWhenFocused, false)
    }

    function test_formatPhone_data()
    {
        return TestData.formatPhone_data()
    }

    function test_formatPhone(data)
    {
        phoneInput.insert(0, data.input)
        tryCompare(phoneInput, "text", data.expectedOutput)
        tryCompare(phoneInput, "cursorPosition", data.expectedCursorPosition)
    }

    function test_modifyPhone_data()
    {
        return TestData.modifyPhone_data()
    }

    function test_modifyPhone(data)
    {
        phoneInput.text = data.input
        tryCompare(phoneInput, "text", data.formatedInput)

        phoneInput.cursorPosition = data.moveCursor
        tryCompare(phoneInput, "cursorPosition", data.moveCursor)

        switch (data.action) {
        case "remove":
            phoneInput.remove(data.moveCursor, data.moveCursor-1)
            break;
        case "insert":
            phoneInput.insert(data.moveCursor, data.text)
        }
        tryCompare(phoneInput, "text", data.newFormatedInput)
        tryCompare(phoneInput, "cursorPosition", data.expectedCursorPosition)
    }

    function test_update_autoFormater()
    {
        phoneInput.autoFormat = false
        phoneInput.text = "7572923"
        compare(phoneInput.text, "7572923")
        phoneInput.autoFormat = true
        compare(phoneInput.text, "757-2923")
    }

    PhoneNumberInput {
        id: phoneInput

        focus: true
        autoFormat: true
        defaultRegion: "US"
    }
}
