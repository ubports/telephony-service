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
#include <QtCore/QDebug>
#include <QtCore/QRegularExpression>
#include <QtCore/QRegularExpressionMatch>

#include "asyoutypeformatter.h"

#include <phonenumbers/phonenumberutil.h>
#include <phonenumbers/region_code.h>


/*!
    \qmltype AsYouTypeFormatter
    \inqmlmodule Ubuntu.Telephony.PhoneNumber 0.1
    \brief The AsYouTypeFormatter is a helper class to format phone numbers

    \b{This component is under heavy development.}

    Example:
    \qml
    Item {
        TextField {
            id: field
            AsYouTypeFormatter {
                id: formatter
                text: field.text
            }

            Binding {
                target: field
                property: "text"
                value: formatter.formattedText
            }
        }
    }
    \endqml
*/


AsYouTypeFormatter::AsYouTypeFormatter(QObject *parent)
    : QObject(parent),
      m_defaultRegionCode(i18n::phonenumbers::RegionCode::GetUnknown()),
      m_enabled(true)
{
    connect(this, SIGNAL(enabledChanged()), SLOT(updateFormattedText()), Qt::QueuedConnection);
    connect(this, SIGNAL(textChanged()), SLOT(updateFormattedText()), Qt::QueuedConnection);
    connect(this, SIGNAL(defaultRegionCodeChanged()), SLOT(updateFormattedText()), Qt::QueuedConnection);
}

AsYouTypeFormatter::~AsYouTypeFormatter()
{
    if (m_formatter) {
        delete m_formatter;
        m_formatter = 0;
    }
}


/*!
  Specifies whether the phone number format is enabled or not.

  \qmlproperty bool enabled
*/
bool AsYouTypeFormatter::enabled() const
{
    return m_enabled;
}

void AsYouTypeFormatter::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;
        Q_EMIT enabledChanged();
    }
}

/*!
  Two letters region code to be used if the number does not provide a country code (+<country-code>).
  These must be provided using ISO 3166-1 two-letter country-code format. The list of the
  codes can be found here: http://www.iso.org/iso/english_country_names_and_code_elements

  \qmlproperty string defaultRegion
*/
QString AsYouTypeFormatter::defaultRegionCode() const
{
    return m_defaultRegionCode;
}

void AsYouTypeFormatter::setDefaultRegionCode(const QString &regionCode)
{
    if (m_defaultRegionCode != regionCode) {
        m_defaultRegionCode = regionCode;
        delete m_formatter;
        m_formatter = 0;
        Q_EMIT defaultRegionCodeChanged();
    }
}

/*!
  Input text to be formatted

  \qmlproperty string text
*/
QString AsYouTypeFormatter::text() const
{
    return m_rawText;
}

void AsYouTypeFormatter::setText(const QString &text)
{
    if (m_rawText != text) {
        m_rawText = text;
        Q_EMIT textChanged();
    }
}

/*!
  Otuput text in a phone number format if the input text is valid.

  \qmlproperty string formattedText
*/
QString AsYouTypeFormatter::formattedText() const
{
    return m_formattedText;
}

/*!
  Clear the input and formatted text
*/
void AsYouTypeFormatter::clear()
{
    m_rawText.clear();
    m_formatter->Clear();
    Q_EMIT textChanged();
}

void AsYouTypeFormatter::updateFormattedText()
{
    static const QRegularExpression numbersReg = QRegularExpression("\\+*\\d+");
    if (!m_enabled) {
        if (!m_formattedText.isEmpty()) {
            m_formattedText.clear();
            Q_EMIT formattedTextChanged();
        }
        return;
    }

    if (m_rawText == m_formattedText) {
        return;
    }

    // Remove all non digits chars
    QRegularExpressionMatchIterator i = numbersReg.globalMatch(m_rawText);
    QString onlyNumbers;
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString word = match.captured();
        onlyNumbers += word;
    }

    // if the number starts with "+" we will use unknown region otherwise we will use the default region
    QString numberRegion = m_defaultRegionCode;
    if (m_rawText.startsWith("+")) {
        numberRegion = i18n::phonenumbers::RegionCode::GetUnknown();
    }

    // destroy current formatter if it was created with a different region
    if (m_formatter && (m_formatterRegionCode != numberRegion)) {
        delete m_formatter;
        m_formatter = 0;
    }

    std::string result;
    QByteArray number(onlyNumbers.toUtf8());

    if (m_formatter) {
        m_formatter->Clear();
    } else {
        i18n::phonenumbers::PhoneNumberUtil *phonenumberUtil = i18n::phonenumbers::PhoneNumberUtil::GetInstance();
        m_formatter = phonenumberUtil->GetAsYouTypeFormatter(numberRegion.toStdString());
        m_formatterRegionCode = numberRegion;
    }

    for(int i = 0, iMax = number.size(); i < iMax; i++) {
        m_formatter->InputDigit(number[i], &result);
    }

    QString newFormattedText = QString::fromStdString(result);
    if (newFormattedText != m_formattedText) {
        m_formattedText = newFormattedText;
        Q_EMIT formattedTextChanged();
    }
}
