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

function reset_item(item)
{
    item.defaultRegion = "US"
    item.autoFormat = true
    item.text = ""
    item.cursorPosition = 0
    item.updateOnlyWhenFocused = false
}

function formatPhone_data()
{
    var data = [];
    data.push({input: "7572923", expectedOutput: "757-2923", expectedCursorPosition: 8})                    // Local number
    data.push({input: "7327572923", expectedOutput: "(732) 757-2923", expectedCursorPosition: 14})          // Coutry number
    data.push({input: "+558187042155", expectedOutput: "+55 81 8704-2155", expectedCursorPosition: 16})     // International number
    data.push({input: "55555555555", expectedOutput: "55555555555", expectedCursorPosition: 11})            // Ivalid number

    return data
}

function modifyPhone_data()
{
    var data = [];
    // Input a number
    data.push({input: "555",  formatedInput: "555", moveCursor: 3,
               action: "insert", text: "5",
               newFormatedInput: "555-5", expectedCursorPosition: 5})

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
