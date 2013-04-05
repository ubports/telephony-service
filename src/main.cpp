/*
 * Copyright (C) 2012-2013 Canonical, Ltd.
 *
 * Authors:
 *  Olivier Tilloy <olivier.tilloy@canonical.com>
 *
 * This file is part of phone-app.
 *
 * phone-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * phone-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Qt
#include <QDebug>
#include <QString>
#include <QTemporaryFile>
#include <QTextStream>

// libc
#include <cerrno>
#include <cstdlib>
#include <cstring>

// local
#include "phoneapplication.h"
#include "config.h"

// Temporarily disable the telepathy folks backend
// as it doesn’t play well with QtFolks.
static void disableTelepathyFolksBackend(QGuiApplication* application)
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
    QGuiApplication::setApplicationName("Phone App");
    PhoneApplication application(argc, argv);

    disableTelepathyFolksBackend(&application);

    if (!application.setup()) {
        return 0;
    }

    return application.exec();
}

