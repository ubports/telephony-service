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
#include <QtDeclarative/QDeclarativeComponent>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeContext>

#include "../modelsectioncounter.h"

class QDeclarativeVisualModel;

class ModelSectionCounterTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testInitialCount_data();
    void testInitialCount();
    void testModelReset();
    void testInsertItem();
    void testRemoveItem();
    void testMoveItem();
    void testChangeItem();

private:
    QDeclarativeVisualModel* createVisualDataModel(QObject* list);
};

Q_DECLARE_METATYPE(QStringListModel*)
Q_DECLARE_METATYPE(ModelSectionCounter::SectionCriteria)

QDeclarativeVisualModel* ModelSectionCounterTest::createVisualDataModel(QObject* list)
{
    /* Instantiate VisualDataModel via a QML snippet because
       QDeclarativeVisualDataModel is not exported by the QtDeclarative library
    */
    QDeclarativeEngine* engine = new QDeclarativeEngine(list);
    engine->rootContext()->setContextProperty("myModel", list);
    QDeclarativeComponent component(engine);
    component.setData("import QtQuick 1.0; VisualDataModel { model: myModel; delegate: Item {} }", QUrl());
    QObject *instance = component.create();
    return (QDeclarativeVisualModel*)instance;
}

void ModelSectionCounterTest::testInitialCount_data()
{
    QTest::addColumn<QStringListModel*>("list");
    QTest::addColumn<QString>("sectionProperty");
    QTest::addColumn<ModelSectionCounter::SectionCriteria>("sectionCriteria");
    QTest::addColumn<int>("sectionCount");


    QTest::newRow("empty list") << new QStringListModel() << "display" << ModelSectionCounter::FullString << 0;
    QTest::newRow("empty list FirstCharacter") << new QStringListModel() << "display" << ModelSectionCounter::FirstCharacter << 0;

    QStringList list;
    list << "a" << "b" << "c";
    QTest::newRow("trivial list") << new QStringListModel(list) << "display" << ModelSectionCounter::FullString << 3;
    QTest::newRow("trivial list FirstCharacter") << new QStringListModel(list) << "display" << ModelSectionCounter::FirstCharacter << 3;

    list.clear();
    list << "My blabla" << "My blabla" << "My blabla";
    QTest::newRow("identical items") << new QStringListModel(list) << "display" << ModelSectionCounter::FullString << 1;
    QTest::newRow("identical items FirstCharacter") << new QStringListModel(list) << "display" << ModelSectionCounter::FirstCharacter << 1;

    list.clear();
    list << "My blabla" << "My blabla" << "Your blabla";
    QTest::newRow("ordered list") << new QStringListModel(list) << "display" << ModelSectionCounter::FullString << 2;
    QTest::newRow("ordered list FirstCharacter") << new QStringListModel(list) << "display" << ModelSectionCounter::FirstCharacter << 2;

    list.clear();
    list << "My blabla" << "Your blabla" << "My blabla";
    QTest::newRow("unordered list") << new QStringListModel(list) << "display" << ModelSectionCounter::FullString << 3;
    QTest::newRow("unordered list FirstCharacter") << new QStringListModel(list) << "display" << ModelSectionCounter::FirstCharacter << 3;

    list.clear();
    list << "My blabla" << "My toto" << "My toto";
    QTest::newRow("ordered list") << new QStringListModel(list) << "display" << ModelSectionCounter::FullString << 2;
    QTest::newRow("ordered list FirstCharacter") << new QStringListModel(list) << "display" << ModelSectionCounter::FirstCharacter << 1;

}

void ModelSectionCounterTest::testInitialCount()
{
    QFETCH(QStringListModel*, list);
    QFETCH(QString, sectionProperty);
    QFETCH(ModelSectionCounter::SectionCriteria, sectionCriteria);
    QFETCH(int, sectionCount);
    QDeclarativeVisualModel* model = createVisualDataModel(list);

    ModelSectionCounter counter;
    counter.setSectionProperty(sectionProperty);
    counter.setSectionCriteria(sectionCriteria);
    counter.setModel(model);

    QCOMPARE(counter.sectionCount(), (unsigned int)sectionCount);
}

void ModelSectionCounterTest::testModelReset()
{
    QStringList list;
    list << "My blabla" << "My blabla" << "Your blabla";
    QStringListModel listModel(list);
    QDeclarativeVisualModel* model = createVisualDataModel(&listModel);

    ModelSectionCounter counter;
    counter.setSectionProperty("display");
    counter.setModel(model);

    QCOMPARE(counter.sectionCount(), (unsigned int)2);

    // create another model and pass it to ModelSectionCounter
    list.clear();
    list << "Your blabla" << "Your blabla" << "Your blabla";
    QStringListModel anotherListModel(list);
    QDeclarativeVisualModel* anotherModel = createVisualDataModel(&anotherListModel);
    counter.setModel(anotherModel);

    QCOMPARE(counter.sectionCount(), (unsigned int)1);
}

void ModelSectionCounterTest::testInsertItem()
{
    QStringList list;
    list << "My blabla" << "My blabla" << "Your blabla";
    QStringListModel listModel(list);
    QDeclarativeVisualModel* model = createVisualDataModel(&listModel);

    ModelSectionCounter counter;
    counter.setSectionProperty("display");
    counter.setModel(model);

    QCOMPARE(counter.sectionCount(), (unsigned int)2);

    // insert one empty string at the beginning of the list
    listModel.insertRows(0, 1);

    QCOMPARE(counter.sectionCount(), (unsigned int)3);

    // insert one empty string at the end of the list
    listModel.insertRows(listModel.rowCount(), 1);

    QCOMPARE(counter.sectionCount(), (unsigned int)4);

}

void ModelSectionCounterTest::testRemoveItem()
{
    QStringList list;
    list << "My blabla" << "My blabla" << "Your blabla";
    QStringListModel listModel(list);
    QDeclarativeVisualModel* model = createVisualDataModel(&listModel);

    ModelSectionCounter counter;
    counter.setSectionProperty("display");
    counter.setModel(model);

    QCOMPARE(counter.sectionCount(), (unsigned int)2);

    // remove first row of the list
    listModel.removeRow(0);

    QCOMPARE(counter.sectionCount(), (unsigned int)2);

    // remove first row of the list
    listModel.removeRow(0);

    QCOMPARE(counter.sectionCount(), (unsigned int)1);

    // remove first row of the list
    listModel.removeRow(0);

    QCOMPARE(counter.sectionCount(), (unsigned int)0);
}

void ModelSectionCounterTest::testMoveItem()
{
    // FIXME: needs to be implemented
}

void ModelSectionCounterTest::testChangeItem()
{
    QStringList list;
    list << "My blabla" << "My blabla" << "Your blabla";
    QStringListModel listModel(list);
    QDeclarativeVisualModel* model = createVisualDataModel(&listModel);

    ModelSectionCounter counter;
    counter.setSectionProperty("display");
    counter.setModel(model);

    QCOMPARE(counter.sectionCount(), (unsigned int)2);

    // change value of second row of the list
    listModel.setData(listModel.index(1), QVariant("Your blabla"), Qt::DisplayRole);

    QCOMPARE(counter.sectionCount(), (unsigned int)2);

    // change value of first row of the list
    listModel.setData(listModel.index(0), QVariant("Your blabla"), Qt::DisplayRole);

    /* The following test fails because QStringListModel::setData() does not
       trigger the emission of QDeclarativeVisualModel::itemsChanged even
       though QStringListModel::dataChanged is emitted. That is because
       QStringListModel does not set any roleNames to itself.
    */
    QEXPECT_FAIL("", "Fails because QStringListModel::setData() does not trigger the emission of QDeclarativeVisualModel::itemsChanged. More explanation in the code.", Continue);
    QCOMPARE(counter.sectionCount(), (unsigned int)1);
}

QTEST_MAIN(ModelSectionCounterTest)
#include "ModelSectionCounterTest.moc"
