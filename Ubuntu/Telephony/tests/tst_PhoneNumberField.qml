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

TestCase {
    id: phoneNumberFieldTest
    name: "PhoneNumberFieldTest"

    function init() {
        phoneField.defaultRegion = "US"
        phoneField.autoFormat = true
        phoneField.text = ""
        phoneField.cursorPosition = 0
        phoneField.updateOnlyWhenFocused = false
    }

    function test_updateOnlyWhenFocused()
    {
        phoneField.updateOnlyWhenFocused = false
        compare(phoneField.updateOnlyWhenFocused, false)
    }

    function test_formatPhone_data()
    {
        var data = [];
        data.push({input: "7572923", expectedOutput: "757-2923", expectedCursorPosition: 8})                    // Local number
        data.push({input: "7327572923", expectedOutput: "(732) 757-2923", expectedCursorPosition: 14})          // Coutry number
        data.push({input: "+558187042155", expectedOutput: "+55 81 8704-2155", expectedCursorPosition: 16})     // International number
        data.push({input: "55555555555", expectedOutput: "55555555555", expectedCursorPosition: 11})            // Ivalid number

        return data
    }

    function test_formatPhone(data)
    {
        phoneField.insert(0, data.input)
        tryCompare(phoneField, "text", data.expectedOutput)
        tryCompare(phoneField, "cursorPosition", data.expectedCursorPosition)
    }

    function test_modifyPhone_data()
    {
        var data = [];
        // change a number and keep the format
        data.push({input: "7327572923",  formatedInput: "(732) 757-2923", moveCursor: 4,
                   action: "remove", text: "",
                   newFormatedInput: "(737) 572-923", expectedCursorPosition: 3})

        // fix a number wrong formatted
        data.push({input: "01234567890",  formatedInput: "01234567890", moveCursor: 10,
                   action: "remove", text: "",
                   newFormatedInput: "(012) 345-6780", expectedCursorPosition: 13})

        data.push({input: "75772923",  formatedInput: "(757) 729-23", moveCursor: 7,
                   action: "remove", text: "",
                   newFormatedInput: "757-2923", expectedCursorPosition: 3})

        return data
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

    PhoneNumberField {
        id: phoneField

        focus: true
        autoFormat: true
        defaultRegion: "US"
    }
}
