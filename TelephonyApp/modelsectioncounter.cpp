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
#include "qdeclarativevisualitemmodel_p.h"

ModelSectionCounter::ModelSectionCounter(QObject *parent) :
    QObject(parent),
    m_sectionCriteria(FullString),
    m_sectionProperty(""),
    m_model(NULL),
    m_sectionCount(0)
{
}

ModelSectionCounter::SectionCriteria ModelSectionCounter::sectionCriteria() const
{
    return m_sectionCriteria;
}

void ModelSectionCounter::setSectionCriteria(ModelSectionCounter::SectionCriteria sectionCriteria)
{
    if (sectionCriteria == m_sectionCriteria) {
        return;
    }

    m_sectionCriteria = sectionCriteria;
    Q_EMIT sectionCriteriaChanged();
    updateSectionCount();
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
    watchSectionPropertyRole();

    Q_EMIT sectionPropertyChanged();
    updateSectionCount();
}

QDeclarativeVisualModel* ModelSectionCounter::model() const
{
    return m_model;
}

void ModelSectionCounter::setModel(QDeclarativeVisualModel* model)
{
    if (model == m_model) {
        return;
    }

    if (m_model != NULL) {
        m_model->disconnect(this);
    }

    m_model = model;
    watchSectionPropertyRole();

    if (model != NULL) {
        connect(model, SIGNAL(itemsMoved(int,int,int)), SLOT(updateSectionCount()));
        connect(model, SIGNAL(itemsChanged(int,int)), SLOT(updateSectionCount()));
        connect(model, SIGNAL(countChanged()), SLOT(updateSectionCount()));
        connect(model, SIGNAL(modelReset()), SLOT(updateSectionCount()));
        connect(model, SIGNAL(createdItem(int,QDeclarativeItem*)), SLOT(updateSectionCount()));
    }

    Q_EMIT modelChanged();
    updateSectionCount();
}

unsigned int ModelSectionCounter::sectionCount() const
{
    return m_sectionCount;
}

QString ModelSectionCounter::sectionString(const QString &value)
{
    if (m_sectionCriteria == FirstCharacter) {
        return value.isEmpty() ? QString() : value.at(0);
    } else {
        return value;
    }
}

void ModelSectionCounter::updateSectionCount()
{
    unsigned int sectionCount = 0;

    if (m_model != NULL) {
        int rowCount = m_model->count();
        QString previousRowString;
        QString currentRowString;

        for (unsigned int i=0; i<rowCount; i++) {
            currentRowString = sectionString(m_model->stringValue(i, m_sectionProperty));
            if (currentRowString != previousRowString || i == 0) {
                sectionCount++;
            }
            previousRowString = currentRowString;
        }
    }

    if (sectionCount != m_sectionCount) {
        m_sectionCount = sectionCount;
        Q_EMIT sectionCountChanged();
    }
}

void ModelSectionCounter::watchSectionPropertyRole()
{
    /* Necessary to have QDeclarativeVisualModel::itemsChanged
       emitted when the value of the section property changes.
       The code is identical to what ListView does internally.
    */
    if (!m_sectionProperty.isEmpty() && m_model != NULL) {
        QList<QByteArray> roles;
        roles << m_sectionProperty.toUtf8();
        m_model->setWatchedRoles(roles);
    }
}
