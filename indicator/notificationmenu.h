/*
 * Copyright (C) 2013-2015 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Pete Woods <pete.woods@canonical.com>
 * Author: Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 */

#ifndef NOTIFICATIONMENU_H_
#define NOTIFICATIONMENU_H_

#include <QString>
#include <QScopedPointer>

class NotificationMenuPriv;

class NotificationMenu {
public:
	NotificationMenu(const QString &id, bool needsResponse = false, bool password = false);
	virtual ~NotificationMenu();
    const QString & id() const;
	const QString & busName() const;
	const QString & response() const;
	const QString & actionPath() const;
	const QString & menuPath() const;
	void clearResponse();
protected:
	QScopedPointer<NotificationMenuPriv> p;
};

#endif /* NOTIFICATIONMENU_H_ */
