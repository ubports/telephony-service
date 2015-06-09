/*
 * Copyright (C) 2015 Canonical, Ltd.
 *
 * This file is part of telephony-service.
 *
 * telephony-service is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-service is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "messagingmenumock.h"
#include "messaging-menu.h"
#include <QDateTime>
#include <QDebug>

MessagingMenuMock *MessagingMenuMock::instance()
{
    static MessagingMenuMock *self = new MessagingMenuMock();
    return self;
}

MessagingMenuMock::MessagingMenuMock(QObject *parent) :
    QObject(parent)
{
}

/*** Mock implementation of messaging messaging-menu functions ***/

/*** MessagingMenuApp ***/
struct _MessagingMenuApp {
    GObject parent_instance;
    gchar *desktopId;
};

G_DEFINE_TYPE(MessagingMenuApp, messaging_menu_app, G_TYPE_OBJECT)

enum MessageProperties
{
    PROP_0,
    PROP_ID,
    PROP_ICON,
    PROP_TITLE,
    PROP_SUBTITLE,
    PROP_BODY,
    PROP_TIME,
    PROP_DRAWS_ATTENTION,
    NUM_PROPERTIES
};

#define PROP_DESKTOP_ID 1

static void messaging_menu_app_set_property (GObject      *object,
                                             guint         prop_id,
                                             const GValue *value,
                                             GParamSpec   *pspec)
{
    qDebug() << __PRETTY_FUNCTION__;
    if (prop_id == PROP_DESKTOP_ID) {
        MessagingMenuApp *app = MESSAGING_MENU_APP (object);
        app->desktopId = g_value_dup_string (value);
    }
}

static void messaging_menu_app_class_init (MessagingMenuAppClass *klass)
{
    qDebug() << __PRETTY_FUNCTION__;
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    object_class->set_property = messaging_menu_app_set_property;

    GParamSpec *prop = g_param_spec_string ("desktop-id",
                                           "Desktop Id",
                                           "The desktop id of the associated application",
                                           NULL,
                                           (GParamFlags)(G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (object_class, PROP_DESKTOP_ID, prop);
}

static void messaging_menu_app_init (MessagingMenuApp *app)
{
    qDebug() << __PRETTY_FUNCTION__;
    Q_UNUSED(app)
    // FIXME: check what needs implementing
}

MessagingMenuApp *messaging_menu_app_new(const gchar *desktop_id)
{
    qDebug() << __PRETTY_FUNCTION__;
    MessagingMenuApp *app = (MessagingMenuApp*)g_object_new (MESSAGING_MENU_TYPE_APP,
                                                             "desktop-id", desktop_id,
                                                             NULL);
    Q_EMIT MessagingMenuMock::instance()->appCreated(app->desktopId);
    return app;
}

void messaging_menu_app_register(MessagingMenuApp *app)
{
    qDebug() << __PRETTY_FUNCTION__;
    Q_EMIT MessagingMenuMock::instance()->appRegistered(app->desktopId);
}

void messaging_menu_app_append_source(MessagingMenuApp *app, const gchar *id, GIcon *icon, const gchar *label)
{
    qDebug() << __PRETTY_FUNCTION__;
    Q_EMIT MessagingMenuMock::instance()->sourceAdded(app->desktopId, id, g_icon_to_string(icon), label);
}

/*** MessagingMenuMessage ***/

struct _MessagingMenuMessage {
    GObject parent;

    gchar *id;
    GIcon *icon;
    gchar *title;
    gchar *subtitle;
    gchar *body;
    gint64 time;
    gboolean draws_attention;
};

typedef GObjectClass MessagingMenuMessageClass;
G_DEFINE_TYPE (MessagingMenuMessage, messaging_menu_message, G_TYPE_OBJECT)

// MessagingMenuMessage's properties
static GParamSpec *properties[NUM_PROPERTIES];

static void messaging_menu_message_get_property(GObject *object,
                                                guint property_id,
                                                GValue *value,
                                                GParamSpec *pspec)
{
    MessagingMenuMessage *msg = MESSAGING_MENU_MESSAGE (object);

    switch (property_id) {
    case PROP_ID:
        g_value_set_string (value, msg->id);
        break;
    case PROP_ICON:
        g_value_set_object (value, msg->icon);
        break;
    case PROP_TITLE:
        g_value_set_string (value, msg->title);
        break;
    case PROP_SUBTITLE:
        g_value_set_string (value, msg->subtitle);
        break;
    case PROP_BODY:
        g_value_set_string (value, msg->body);
        break;
    case PROP_TIME:
        g_value_set_int64 (value, msg->time);
        break;
    case PROP_DRAWS_ATTENTION:
        g_value_set_boolean (value, msg->draws_attention);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void messaging_menu_message_set_property(GObject *object,
                                                guint property_id,
                                                const GValue *value,
                                                GParamSpec *pspec)
{
    qDebug() << __PRETTY_FUNCTION__;
    MessagingMenuMessage *msg = MESSAGING_MENU_MESSAGE (object);

    switch (property_id) {
    case PROP_ID:
        msg->id = g_value_dup_string (value);
        break;
    case PROP_ICON:
        msg->icon = (GIcon*)g_value_dup_object (value);
        break;
    case PROP_TITLE:
        msg->title = g_value_dup_string (value);
        break;
    case PROP_SUBTITLE:
        msg->subtitle = g_value_dup_string (value);
        break;
    case PROP_BODY:
        msg->body = g_value_dup_string (value);
        break;
    case PROP_TIME:
        msg->time = g_value_get_int64 (value);
        break;
    case PROP_DRAWS_ATTENTION:
       msg->draws_attention = g_value_get_boolean (value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}
static void messaging_menu_message_class_init(MessagingMenuMessageClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = messaging_menu_message_get_property;
    object_class->set_property = messaging_menu_message_set_property;

    properties[PROP_ID] = g_param_spec_string ("id", "Id",
                                               "Unique id of the message",
                                               NULL,
                                               (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
    properties[PROP_ICON] = g_param_spec_object ("icon", "Icon",
                                                 "Icon of the message",
                                                 G_TYPE_ICON,
                                                 (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
    properties[PROP_TITLE] = g_param_spec_string ("title", "Title",
                                                  "Title of the message",
                                                  NULL,
                                                  (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
    properties[PROP_SUBTITLE] = g_param_spec_string ("subtitle", "Subtitle",
                                                     "Subtitle of the message",
                                                     NULL,
                                                     (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
    properties[PROP_BODY] = g_param_spec_string ("body", "Body",
                                                 "First lines of the body of the message",
                                                 NULL,
                                                 (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
    properties[PROP_TIME] = g_param_spec_int64 ("time", "Time",
                                                "Time the message was sent, in microseconds", 0, G_MAXINT64, 0,
                                                (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
    properties[PROP_DRAWS_ATTENTION] = g_param_spec_boolean ("draws-attention", "Draws attention",
                                                               "Whether the message should draw attention",
                                                               TRUE,
                                                               (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
    g_object_class_install_properties (klass, NUM_PROPERTIES, properties);

    g_signal_new ("activate",
                  MESSAGING_MENU_TYPE_MESSAGE,
                  (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED),
                  0,
                  NULL, NULL,
                  g_cclosure_marshal_generic,
                  G_TYPE_NONE, 2,
                  G_TYPE_STRING,
                  G_TYPE_VARIANT);
}

static void messaging_menu_message_init(MessagingMenuMessage *self)
{
    self->draws_attention = TRUE;
}

MessagingMenuMessage *messaging_menu_message_new(const gchar *id, GIcon *icon, const gchar *title, const gchar *subtitle, const gchar *body, gint64 time)
{
    qDebug() << __PRETTY_FUNCTION__;
    MessagingMenuMessage *message = (MessagingMenuMessage*) g_object_new(MESSAGING_MENU_TYPE_MESSAGE,
                                                                         "id", id,
                                                                         "icon", icon,
                                                                         "title", title,
                                                                         "subtitle", subtitle,
                                                                         "body", body,
                                                                         "time", time,
                                                                         NULL);
    Q_EMIT MessagingMenuMock::instance()->messageCreated(QString(message->id),
                                                         QString(g_icon_to_string(message->icon)),
                                                         QString(message->title),
                                                         QString(message->subtitle),
                                                         QString(message->body),
                                                         QDateTime::fromMSecsSinceEpoch(time / 1000));

    return message;
}

void messaging_menu_message_add_action(MessagingMenuMessage *msg, const gchar *id, const gchar *label, const GVariantType *parameter_type, GVariant *parameter_hint)
{
    qDebug() << __PRETTY_FUNCTION__;
    Q_EMIT MessagingMenuMock::instance()->actionAdded(msg->id, id, label);
}

void messaging_menu_app_remove_message_by_id(MessagingMenuApp *app, const gchar *id)
{
    qDebug() << __PRETTY_FUNCTION__;
    Q_EMIT MessagingMenuMock::instance()->messageRemoved(app->desktopId, id);
}

const gchar *messaging_menu_message_get_id(MessagingMenuMessage *msg)
{
    qDebug() << __PRETTY_FUNCTION__;
    return msg->id;
}

void messaging_menu_app_append_message(MessagingMenuApp *app, MessagingMenuMessage *msg, const gchar *source_id, gboolean notify)
{
    qDebug() << __PRETTY_FUNCTION__;
    Q_EMIT MessagingMenuMock::instance()->messageAdded(QString(app->desktopId), QString(msg->id), QString(source_id), (bool)notify);
}

