/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Florian Boucault <florian.boucault@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "modelsectioncounter.h"

ModelSectionCounter::ModelSectionCounter(QObject *parent) :
    QObject(parent),
    m_sectionProperty(""),
    m_model(NULL),
    m_sectionCount(0)
{
}

QString ModelSectionCounter::sectionProperty() const
{
    return m_sectionProperty;
}

void ModelSectionCounter::setSectionProperty(const QString &sectionProperty)
{
    if (sectionProperty == m_sectionProperty) {
        return;
    }

    m_sectionProperty = sectionProperty;
    Q_EMIT sectionPropertyChanged();
    updateSectionCount();
}

QAbstractListModel* ModelSectionCounter::model() const
{
    return m_model;
}

void ModelSectionCounter::setModel(QAbstractListModel* model)
{
    if (model == m_model) {
        return;
    }

    // FIXME: dis/connect updateSectionCount() to model's change signals

    Q_EMIT modelChanged();
    updateSectionCount();
}

unsigned int ModelSectionCounter::sectionCount() const
{
    return m_sectionCount;
}

void ModelSectionCounter::updateSectionCount()
{
    unsigned int sectionCount = 0;
    int rowCount = m_model->rowCount();
    QString previousRowCriteria;
    QString currentRowCriteria;

    // FIXME: add support for more than just QStringListModel
    for (unsigned int i=0; i<rowCount; i++) {
        QVariant data = m_model->data(m_model->index(i));
        currentRowCriteria = data.toString()[0];
        if (currentRowCriteria != previousRowCriteria) {
            sectionCount++;
        }
        previousRowCriteria = currentRowCriteria;
    }

    if (sectionCount != m_sectionCount) {
        m_sectionCount = sectionCount;
        Q_EMIT sectionCountChanged();
    }
}
