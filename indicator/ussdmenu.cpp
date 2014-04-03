/*
 * Copyright (C) 2013 Canonical, Ltd.
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

#include "ussdmenu.h"
#include <gio/gio.h>

#include <QString>
#include <QDebug>

static const QString USSD_ACTION_PATH("/action/%1");
static const QString USSD_MENU_PATH("/menu/%1");

class USSDMenuPriv {
public:
	USSDMenuPriv() :
			m_connection(g_bus_get_sync(G_BUS_TYPE_SESSION, NULL,
			NULL)), m_exportedActionGroupId(0), m_exportedMenuModelId(0) {
	}

	~USSDMenuPriv() {
		g_object_unref(m_connection);
	}

	static void responseChangedCallback(GAction *responseAction,
			GVariant *variant, gpointer userData) {
		USSDMenuPriv *self(reinterpret_cast<USSDMenuPriv*>(userData));
		self->m_response = QString::fromUtf8(g_variant_get_string(variant, 0));
        }

	GDBusConnection *m_connection;
	QString m_busName;
	QString m_actionPath;
	QString m_menuPath;
	unsigned int m_exportedActionGroupId;
	unsigned int m_exportedMenuModelId;
	QString m_response;
};

USSDMenu::USSDMenu(bool needsResponse) :
		p(new USSDMenuPriv()) {
	int exportrev;

	p->m_busName = QString::fromUtf8(
			g_dbus_connection_get_unique_name(p->m_connection));

	// menu
	GMenu *menu(g_menu_new());

	GMenuItem *ussdItem(g_menu_item_new("", "notifications.ussd"));
        if (needsResponse) {
            g_menu_item_set_attribute_value(ussdItem, "x-canonical-type",
                            g_variant_new_string("com.canonical.snapdecision.textfield"));
            g_menu_item_set_attribute_value(ussdItem, "x-echo-mode-password",
                            g_variant_new_boolean(false));
            g_menu_append_item(menu, ussdItem);
        }

	// actions
	GActionGroup *actions(G_ACTION_GROUP(g_simple_action_group_new()));
	GAction *ussdAction(G_ACTION(
			g_simple_action_new_stateful("ussd", G_VARIANT_TYPE_STRING,
					g_variant_new_string(""))));

	g_signal_connect(G_OBJECT(ussdAction), "change-state",
			G_CALLBACK(USSDMenuPriv::responseChangedCallback),
			reinterpret_cast<gpointer>(p.data()));

	g_action_map_add_action(G_ACTION_MAP(actions), ussdAction);

	/* Export the actions group.  If we can't get a name, keep trying to
	   use increasing numbers.  There is possible races on fast import/exports.
	   They're rare, but worth protecting against. */
	exportrev = 0;
	do {
		exportrev++;
		p->m_actionPath = USSD_ACTION_PATH.arg(exportrev);
		p->m_exportedActionGroupId = g_dbus_connection_export_action_group(
				p->m_connection, p->m_actionPath.toUtf8().data(), actions, NULL);
	} while (p->m_exportedActionGroupId == 0 && exportrev < 128);

	/* Export the menu.  If we can't get a name, keep trying to
	   use increasing numbers.  There is possible races on fast import/exports.
	   They're rare, but worth protecting against. */
	exportrev = 0;
	do {
		exportrev++;
		p->m_menuPath = USSD_MENU_PATH.arg(exportrev);
		p->m_exportedMenuModelId = g_dbus_connection_export_menu_model(
				p->m_connection, p->m_menuPath.toUtf8().data(),
				G_MENU_MODEL(menu), NULL);
	} while (p->m_exportedMenuModelId == 0 && exportrev < 128);

	/* Unref the objects as a reference is maintained by the fact that they're
	   exported onto the bus. */
	g_object_unref(menu);
	g_object_unref(ussdItem);

	g_object_unref(actions);
	g_object_unref(ussdAction);
}

USSDMenu::~USSDMenu() {
	g_dbus_connection_unexport_action_group(p->m_connection,
			p->m_exportedActionGroupId);
	g_dbus_connection_unexport_menu_model(p->m_connection,
			p->m_exportedMenuModelId);
}

const QString & USSDMenu::busName() const {
	return p->m_busName;
}

const QString & USSDMenu::response() const {
	return p->m_response;
}

const QString & USSDMenu::actionPath() const {
	return p->m_actionPath;
}

const QString & USSDMenu::menuPath() const {
	return p->m_menuPath;
}

void USSDMenu::clearResponse() {
	p->m_response.clear();
}
