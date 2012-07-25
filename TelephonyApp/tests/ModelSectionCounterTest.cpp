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

#include <QtCore/QObject>
#include <QtTest/QtTest>
#include <QtGui/QStringListModel>

#include "../modelsectioncounter.h"

class ModelSectionCounterTest : public QObject
{
    Q_OBJECT

private slots:
    void testCount_data();
    void testCount();
};

Q_DECLARE_METATYPE(QStringListModel*)

void ModelSectionCounterTest::testCount_data()
{
    QTest::addColumn<QStringListModel*>("list");
    QTest::addColumn<QString>("sectionProperty");
    QTest::addColumn<int>("sectionCount");


    QTest::newRow("empty list") << new QStringListModel() << "modelData" << 0;

    QStringList list;
    list << "a" << "b" << "c";
    QTest::newRow("trivial list") << new QStringListModel(list) << "modelData" << 3;

    list.clear();
    list << "My blabla" << "My blabla" << "My blabla";
    QTest::newRow("identical items") << new QStringListModel(list) << "modelData" << 1;

    list.clear();
    list << "My blabla" << "My blabla" << "Your blabla";
    QTest::newRow("ordered list") << new QStringListModel(list) << "modelData" << 2;

    list.clear();
    list << "My blabla" << "Your blabla" << "My blabla";
    QTest::newRow("unordered list") << new QStringListModel(list) << "modelData" << 3;
}

void ModelSectionCounterTest::testCount()
{
    ModelSectionCounter counter;

    QFETCH(QStringListModel*, list);
    QFETCH(QString, sectionProperty);
    QFETCH(int, sectionCount);

    counter.setModel(list);
    counter.setSectionProperty(sectionProperty);

    QCOMPARE(counter.sectionCount(), (unsigned int)sectionCount);
}

QTEST_MAIN(ModelSectionCounterTest)
#include "ModelSectionCounterTest.moc"
