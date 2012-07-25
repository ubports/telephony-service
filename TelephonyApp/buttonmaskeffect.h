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

#ifndef BUTTONMASKEFFECT_H
#define BUTTONMASKEFFECT_H

#include <QtGui/QGraphicsEffect>
#include <QtGui/QGraphicsItem>

class ButtonMaskEffect : public QGraphicsEffect
{
    Q_OBJECT

    Q_PROPERTY(QObject* mask READ mask WRITE setMask NOTIFY maskChanged)
    Q_PROPERTY(qreal gradientStrength READ gradientStrength WRITE setGradientStrength NOTIFY gradientStrengthChanged)
    Q_PROPERTY(bool dark READ dark WRITE setDark NOTIFY darkChanged)

public:
    explicit ButtonMaskEffect(QObject *parent = 0);

    // getters
    QObject* mask() const;
    qreal gradientStrength() const;
    bool dark() const;

    // setters
    void setMask(QObject* mask);
    void setGradientStrength(qreal gradientStrength);
    void setDark(bool dark);

Q_SIGNALS:
    void maskChanged(QObject* mask);
    void gradientStrengthChanged(qreal gradientStrength);
    void darkChanged(bool dark);

protected:
    void draw(QPainter *painter);
    void sourceChanged(ChangeFlags flags);

private:
    QGraphicsItem* m_mask;
    qreal m_gradientStrength;
    bool m_dark;

    // caching of intermediary renderings
    QPixmap m_cachedPixmap;
};

#endif // BUTTONMASKEFFECT_H
