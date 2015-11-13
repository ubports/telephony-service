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

#include "notificationmenu.h"
#include <gio/gio.h>

#include <QString>
#include <QDebug>

static const QString ACTION_PATH("/action/%1");
static const QString MENU_PATH("/menu/%1");

class NotificationMenuPriv {
public:
	NotificationMenuPriv() :
			m_connection(g_bus_get_sync(G_BUS_TYPE_SESSION, NULL,
			NULL)), m_exportedActionGroupId(0), m_exportedMenuModelId(0) {
	}

	~NotificationMenuPriv() {
		g_object_unref(m_connection);
	}

	static void responseChangedCallback(GAction *responseAction,
			GVariant *variant, gpointer userData) {
		NotificationMenuPriv *self(reinterpret_cast<NotificationMenuPriv*>(userData));
		self->m_response = QString::fromUtf8(g_variant_get_string(variant, 0));
        }

	GDBusConnection *m_connection;
    QString m_id;
    QString m_busName;
	QString m_actionPath;
	QString m_menuPath;
	unsigned int m_exportedActionGroupId;
	unsigned int m_exportedMenuModelId;
	QString m_response;
};

NotificationMenu::NotificationMenu(const QString &id, bool needsResponse, bool password) :
        p(new NotificationMenuPriv()) {
	int exportrev;

    p->m_id = id;
	p->m_busName = QString::fromUtf8(
			g_dbus_connection_get_unique_name(p->m_connection));

	// menu
	GMenu *menu(g_menu_new());

	GMenuItem *item(g_menu_item_new("", QString("notifications.%1").arg(id).toLatin1().data()));
        if (needsResponse) {
            g_menu_item_set_attribute_value(item, "x-canonical-type",
                            g_variant_new_string("com.canonical.snapdecision.textfield"));
            g_menu_item_set_attribute_value(item, "x-echo-mode-password",
                            g_variant_new_boolean(password));
            g_menu_append_item(menu, item);
        }

	// actions
	GActionGroup *actions(G_ACTION_GROUP(g_simple_action_group_new()));
	GAction *action(G_ACTION(
			g_simple_action_new_stateful(id.toLatin1().data(), G_VARIANT_TYPE_STRING,
					g_variant_new_string(""))));

	g_signal_connect(G_OBJECT(action), "change-state",
			G_CALLBACK(NotificationMenuPriv::responseChangedCallback),
			reinterpret_cast<gpointer>(p.data()));

	g_action_map_add_action(G_ACTION_MAP(actions), action);

	/* Export the actions group.  If we can't get a name, keep trying to
	   use increasing numbers.  There is possible races on fast import/exports.
	   They're rare, but worth protecting against. */
	exportrev = 0;
	do {
		exportrev++;
		p->m_actionPath = ACTION_PATH.arg(exportrev);
		p->m_exportedActionGroupId = g_dbus_connection_export_action_group(
				p->m_connection, p->m_actionPath.toUtf8().data(), actions, NULL);
	} while (p->m_exportedActionGroupId == 0 && exportrev < 128);

	/* Export the menu.  If we can't get a name, keep trying to
	   use increasing numbers.  There is possible races on fast import/exports.
	   They're rare, but worth protecting against. */
	exportrev = 0;
	do {
		exportrev++;
		p->m_menuPath = MENU_PATH.arg(exportrev);
		p->m_exportedMenuModelId = g_dbus_connection_export_menu_model(
				p->m_connection, p->m_menuPath.toUtf8().data(),
				G_MENU_MODEL(menu), NULL);
	} while (p->m_exportedMenuModelId == 0 && exportrev < 128);

	/* Unref the objects as a reference is maintained by the fact that they're
	   exported onto the bus. */
	g_object_unref(menu);
	g_object_unref(item);

	g_object_unref(actions);
	g_object_unref(action);
}

NotificationMenu::~NotificationMenu() {
	g_dbus_connection_unexport_action_group(p->m_connection,
			p->m_exportedActionGroupId);
	g_dbus_connection_unexport_menu_model(p->m_connection,
                                          p->m_exportedMenuModelId);
}

const QString &NotificationMenu::id() const
{
    return p->m_id;
}

const QString & NotificationMenu::busName() const {
	return p->m_busName;
}

const QString & NotificationMenu::response() const {
	return p->m_response;
}

const QString & NotificationMenu::actionPath() const {
	return p->m_actionPath;
}

const QString & NotificationMenu::menuPath() const {
	return p->m_menuPath;
}

void NotificationMenu::clearResponse() {
	p->m_response.clear();
}
