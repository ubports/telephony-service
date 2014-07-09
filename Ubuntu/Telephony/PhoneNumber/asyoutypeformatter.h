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

#ifndef TELEPHONY_PHONENUMBER_ASYOUTYPEFORMATTER_H
#define TELEPHONY_PHONENUMBER_ASYOUTYPEFORMATTER_H

#include <QtCore/QObject>
#include <QtCore/QVariantMap>

#include <phonenumbers/asyoutypeformatter.h>

class AsYouTypeFormatter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QString defaultRegionCode READ defaultRegionCode WRITE setDefaultRegionCode NOTIFY defaultRegionCodeChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString formattedText READ formattedText NOTIFY formattedTextChanged)

public:
    AsYouTypeFormatter(QObject *parent = 0);
    ~AsYouTypeFormatter();

    bool enabled() const;
    void setEnabled(bool enabled);

    QString defaultRegionCode() const;
    void setDefaultRegionCode(const QString &regionCode);

    QString text() const;
    void setText(const QString &text);

    QString formattedText() const;

public Q_SLOTS:
    void clear();
    QVariantMap formatText(const QString &text, int cursorPosition);

private Q_SLOTS:
    void updateFormattedText();


Q_SIGNALS:
    void textChanged();
    void formattedTextChanged();
    void defaultRegionCodeChanged();
    void enabledChanged();

private:
    i18n::phonenumbers::AsYouTypeFormatter *m_formatter;
    bool m_enabled;
    QString m_rawText;
    QString m_formattedText;
    QString m_defaultRegionCode;
    QString m_formatterRegionCode;

    QString formatTextImpl(const QString &text, int *cursorPosition);
};

#endif //TELEPHONY_PHONENUMBER_ASYOUTYPEFORMATTER_H
