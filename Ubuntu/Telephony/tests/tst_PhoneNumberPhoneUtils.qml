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
import Ubuntu.Telephony.PhoneNumber 0.1 as PhoneNumber

TestCase {
    id: phoneNumberPhoneUtils
    name: "phoneNumberPhoneUtils"

    function test_formatPhone_data()
    {
        var data = [];
        data.push({input: "6681800", expectedOutput: "668-1800"})                    // Local number
        data.push({input: "7327572923", expectedOutput: "(732) 757-2923"})           // Coutry number
        data.push({input: "+558187042155", expectedOutput: "+55 81 8704-2155"})      // International number
        data.push({input: "55555555555", expectedOutput: "55555555555"})             // Ivalid number
        data.push({input: "*144", expectedOutput: "*144"})                           // Special number
        data.push({input: "#123#", expectedOutput: "#123#"})                         // Operators command
        return data
    }

    function test_formatPhone(data)
    {
        var formatted = PhoneNumber.PhoneUtils.format(data.input, "US")
        compare(formatted, data.expectedOutput)
    }
}
