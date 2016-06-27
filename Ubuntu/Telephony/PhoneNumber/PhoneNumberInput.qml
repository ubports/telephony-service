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

import QtQuick 2.4
import Ubuntu.Components 1.3
import Ubuntu.Telephony.PhoneNumber 0.1
import "PhoneNumber.js" as PhoneNumberJS

/*!
    \qmltype PhoneNumberInput
    \inqmlmodule Ubuntu.Telephony.PhoneNumberPhoneNumberJS 0.1
    \brief The PhoneNumberField element allows to format a phone-number as you type
    The PhoneNumberInput uses TextInput as base class

    \b{This component is under heavy development.}

    Example:
    \qml
    Item {
        PhoneNumberInput {
            autoFormat: true
            defaultRegion: "US"
        }
    \endqml
*/
TextInput {
    id: phoneNumberInput

    /*!
      Specifies whether the phone number format is enabled or not.

      \qmlproperty bool autoFormat
    */
    property alias autoFormat: formatter.enabled

    /*!
      Two letters region code to be used if the number does not provide a country code (+<country-code>).
      These must be provided using ISO 3166-1 two-letter country-code format. The list of the
      codes can be found here: http://www.iso.org/iso/english_country_names_and_code_elements

      \qmlproperty string defaultRegion
    */
    property alias defaultRegion: formatter.defaultRegionCode

    /*!
      Specifies if the autoformat should format the text even if the field does not have focus
      Default value is true

      \qmlproperty bool autoFormat
    */
    property bool updateOnlyWhenFocused: true

    AsYouTypeFormatter {
        id: formatter
    }

    onTextChanged: PhoneNumberJS.onTextChange(phoneNumberInput, formatter)
    onAutoFormatChanged: {
        if (autoFormat) {
            PhoneNumberJS.onTextChange(phoneNumberInput, formatter)
        }
    }
}
