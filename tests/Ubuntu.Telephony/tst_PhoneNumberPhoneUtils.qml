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

import QtQuick 2.2
import QtTest 1.0
import Ubuntu.Telephony.PhoneNumber 0.1 as PhoneNumber

TestCase {
    id: phoneNumberPhoneUtils
    name: "phoneNumberPhoneUtils"

    function test_formatPhone_data()
    {
        var data = [];
        data.push({input: "6681800", expectedOutput: "668-1800"})                    // Local number
        data.push({input: "7327572923", expectedOutput: "(732) 757-2923"})           // Country number
        data.push({input: "+558187042155", expectedOutput: "+55 81 8704-2155"})      // International number
        data.push({input: "55555555555", expectedOutput: "55555555555"})             // Invalid number
        data.push({input: "*144", expectedOutput: "*144"})                           // Special number
        data.push({input: "#123#", expectedOutput: "#123#"})                         // Operators command
        return data
    }

    function test_formatPhone(data)
    {
        var formatted = PhoneNumber.PhoneUtils.format(data.input, "US")
        compare(formatted, data.expectedOutput)
    }

    function test_locale()
    {
        var localeName = Qt.locale().name
        compare(PhoneNumber.PhoneUtils.defaultRegion, localeName.substr(localeName.length - 2, 2))
    }

    function test_matchPhone_data()
    {
        var data = [];
        data.push({text: "my 1rst phone number 617-688-0034, ..",
                      expectedMatches: ["617-688-0034"]})    // Local number
        data.push({text: "my 1rst phone number 650 253 0000, ..",
                      expectedMatches: ["650 253 0000"]})    // Local number
        data.push({text: "my phnle number 7327572923, please call me",
                      expectedMatches: ["7327572923"]})      // Country number
        data.push({text: "my international number +5581987042155, please call me",
                      expectedMatches: ["+5581987042155"]})   // International number
        data.push({text: "this is an invalid number 55555555555, yes yes it is",
                      expectedMatches: []})                  // Invalid number
        data.push({text: "could you call me between 15h30-16h yes?",
                      expectedMatches: []})                  // Invalid number
        data.push({text: "could you call me between at extension *144 yes?",
                      expectedMatches: []})                  // Special number
        data.push({text: "my operator number: #123#, yes",
                      expectedMatches: []})                  // Operators command
        return data
    }

    function compareMatches(matches1, matches2)
    {
        compare(matches1.length, matches2.length)
        for (var i = 0; i < matches1.length; ++i) {
            compare(matches1[i], matches2[i])
        }
    }

    function test_matchPhone(data)
    {
        var actualMatches = PhoneNumber.PhoneUtils.matchInText(data.text, "US")
        compareMatches(actualMatches, data.expectedMatches)
    }
}

