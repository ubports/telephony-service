/*
 * Copyright (C) 2012 Canonical, Ltd.
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

#include "buttonmaskeffect.h"

#include <QtGui/QPainter>
#include <QtGui/QStyleOptionGraphicsItem>

ButtonMaskEffect::ButtonMaskEffect(QObject *parent) :
    QGraphicsEffect(parent),
    m_gradientStrength(1.0),
    m_dark(false),
    m_mask(NULL)
{
}

qreal ButtonMaskEffect::gradientStrength() const
{
    return m_gradientStrength;
}

void ButtonMaskEffect::setGradientStrength(qreal gradientStrength)
{
    if (gradientStrength == m_gradientStrength) {
        return;
    }

    m_gradientStrength = gradientStrength;
    m_cachedPixmap = QPixmap();
    update();
    Q_EMIT gradientStrengthChanged(gradientStrength);
}

bool ButtonMaskEffect::dark() const
{
    return m_dark;
}

void ButtonMaskEffect::setDark(bool dark)
{
    if (dark == m_dark) {
        return;
    }

    m_dark = dark;
    m_cachedPixmap = QPixmap();
    update();
    Q_EMIT darkChanged(dark);
}

QObject* ButtonMaskEffect::mask() const
{
    return dynamic_cast<QObject*>(m_mask);
}

void ButtonMaskEffect::setMask(QObject* mask)
{
    QGraphicsItem* typedMask = dynamic_cast<QGraphicsItem*>(mask);
    if (typedMask == m_mask) {
        return;
    }

    m_mask = typedMask;
    m_cachedPixmap = QPixmap();
    update();
    Q_EMIT maskChanged(mask);
}


void ButtonMaskEffect::draw(QPainter *painter)
{
    QPoint offset;
    const QPixmap pixmap = sourcePixmap(Qt::DeviceCoordinates, &offset, QGraphicsEffect::NoPad);

    if (m_cachedPixmap.isNull()) {
        // copy the source into m_cachedPixmap
        m_cachedPixmap = pixmap.copy();
        QPainter tintedPainter;
        tintedPainter.begin(&m_cachedPixmap);
        tintedPainter.setRenderHints(painter->renderHints());

        // apply vertical gradient to m_cachedPixmap
        QLinearGradient gradient(0, 0, 0, pixmap.height());
        gradient.setColorAt(0.0, QColor(255, 255, 255, m_gradientStrength * 255));
        gradient.setColorAt(1.0, QColor(0, 0, 0, m_gradientStrength * 255));
        if (m_dark) {
            tintedPainter.setCompositionMode(QPainter::CompositionMode_Multiply);
        } else {
            tintedPainter.setCompositionMode(QPainter::CompositionMode_Overlay);
        }
        tintedPainter.fillRect(m_cachedPixmap.rect(), gradient);

        // apply mask to m_cachedPixmap
        QPixmap maskPixmap(pixmap.size());
        maskPixmap.fill(Qt::transparent);
        QPainter maskPainter;
        maskPainter.begin(&maskPixmap);
        maskPainter.setRenderHints(painter->renderHints());
        QStyleOptionGraphicsItem option;
        m_mask->paint(&maskPainter, &option);
        maskPainter.end();

        tintedPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        tintedPainter.drawPixmap(0, 0, maskPixmap);
        tintedPainter.end();
    }

    QTransform previousTransform = painter->worldTransform();
    painter->setWorldTransform(QTransform());
    painter->drawPixmap(offset, m_cachedPixmap);
    painter->setWorldTransform(previousTransform);
}

void ButtonMaskEffect::sourceChanged(ChangeFlags flags)
{
    m_cachedPixmap = QPixmap();
}
