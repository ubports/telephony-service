/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Olivier Tilloy <olivier.tilloy@canonical.com>
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

// Qt
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QString>
#include <QtCore/QTemporaryFile>
#include <QtCore/QTextStream>
#include <QtCore/QUrl>
#include <QtDeclarative/QDeclarativeComponent>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeView>
#include <QtGui/QApplication>

// libc
#include <cerrno>
#include <cstdlib>
#include <cstring>

// local
#include "config.h"

// inspired by qmlviewer’s QDeclarativeViewer::loadDummyDataFiles(…)
static void loadDummyDataFiles(QDeclarativeView* view)
{
    QDir dir(telephonyAppDirectory() + "/dummydata", "*.qml");
    Q_FOREACH(const QString& qmlFile, dir.entryList()) {
        const QString filePath = dir.filePath(qmlFile);
        QDeclarativeComponent comp(view->engine(), filePath);
        QObject* dummyData = comp.create();
        if(comp.isError()) {
            Q_FOREACH(const QDeclarativeError &error, comp.errors()) {
                qWarning() << error;
            }
        }
        if (dummyData) {
            qDebug() << "Loaded dummy data:" << filePath;
            QString propertyName = qmlFile;
            propertyName.chop(4);
            view->rootContext()->setContextProperty(propertyName, dummyData);
            dummyData->setParent(view);
        }
    }
}

// Temporarily disable the telepathy folks backend
// as it doesn’t play well with QtFolks.
static void disableTelepathyFolksBackend(QApplication* application)
{
    QTemporaryFile* temp = new QTemporaryFile(application);
    if (temp->open()) {
        QTextStream out(temp);
        out << "[telepathy]\n";
        out << "enabled=false\n";
        temp->close();
        if (setenv("FOLKS_BACKEND_STORE_KEY_FILE_PATH",
                   temp->fileName().toUtf8().constData(), 1) != 0) {
            qWarning() << "Failed to disable Telepathy Folks backend:"
                       << strerror(errno);
        }
    } else {
        qWarning() << "Failed to disable Telepathy Folks backend:"
                   << temp->errorString();
    }
}

int main(int argc, char** argv)
{
    QApplication application(argc, argv);
    application.setApplicationName("Telephony App");
    disableTelepathyFolksBackend(&application);
    QDeclarativeView view;
    loadDummyDataFiles(&view);
    QUrl source(telephonyAppDirectory() + "/telephony-app.qml");
    view.setSource(source);
    view.show();
    return application.exec();
}

