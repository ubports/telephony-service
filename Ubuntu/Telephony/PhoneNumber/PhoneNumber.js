/*
 * Copyright (C) 2014 Canonical, Ltd.
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

function onTextChange(phoneNumberItem, formatter)
{
    if (phoneNumberItem.text === "") {
        return;
    }

    if (phoneNumberItem.autoFormat &&
        (!phoneNumberItem.updateOnlyWhenFocused || phoneNumberItem.activeFocus)) {
        var result = formatter.formatText(phoneNumberItem.text, phoneNumberItem.cursorPosition)

        if (result.text !== phoneNumberItem.text) {
            phoneNumberItem.text = result.text
            phoneNumberItem.cursorPosition = result.pos
        }
    }
}
