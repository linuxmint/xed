/*
 * xed-taglist-plugin.h
 *
 * Copyright (C) 2002-2005 - Paolo Maggi
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * Modified by the xed Team, 2002-2005. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "xed-taglist-plugin.h"
#include "xed-taglist-plugin-panel.h"
#include "xed-taglist-plugin-parser.h"

#include <glib/gi18n-lib.h>
#include <gmodule.h>
#include <libpeas/peas-activatable.h>

#include <xed/xed-window.h>
#include <xed/xed-debug.h>

#define XED_TAGLIST_PLUGIN_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), XED_TYPE_TAGLIST_PLUGIN, XedTaglistPluginPrivate))

struct _XedTaglistPluginPrivate
{
	GtkWidget *window;

    GtkWidget *taglist_panel;
};

static void peas_activatable_iface_init (PeasActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (XedTaglistPlugin,
                                xed_taglist_plugin,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (PEAS_TYPE_ACTIVATABLE,
                                                               peas_activatable_iface_init) \
                                                                                            \
                                _xed_taglist_plugin_panel_register_type (type_module);    \
)

enum
{
    PROP_0,
    PROP_OBJECT
};

static void
xed_taglist_plugin_init (XedTaglistPlugin *plugin)
{
	plugin->priv = XED_TAGLIST_PLUGIN_GET_PRIVATE (plugin);

	xed_debug_message (DEBUG_PLUGINS, "XedTaglistPlugin initializing");
}

static void
xed_taglist_plugin_dispose (GObject *object)
{
	XedTaglistPlugin *plugin = XED_TAGLIST_PLUGIN (object);

	xed_debug_message (DEBUG_PLUGINS, "XedTaglistPlugin disposing");

    if (plugin->priv->window != NULL)
    {
        g_object_unref (plugin->priv->window);
        plugin->priv->window = NULL;
    }

    G_OBJECT_CLASS (xed_taglist_plugin_parent_class)->dispose (object);
}

static void
xed_taglist_plugin_finalize (GObject *object)
{
    xed_debug_message (DEBUG_PLUGINS, "XedTaglistPlugin finalizing");

	free_taglist ();

	G_OBJECT_CLASS (xed_taglist_plugin_parent_class)->finalize (object);
}

static void
xed_taglist_plugin_activate (PeasActivatable *activatable)
{
    XedTaglistPluginPrivate *priv;
    XedWindow *window;
	XedPanel *side_panel;
	gchar *data_dir;

	xed_debug (DEBUG_PLUGINS);

    priv = XED_TAGLIST_PLUGIN (activatable)->priv;
    window = XED_WINDOW (priv->window);
	side_panel = xed_window_get_side_panel (window);

	data_dir = peas_extension_base_get_data_dir (PEAS_EXTENSION_BASE (activatable));
    priv->taglist_panel = xed_taglist_plugin_panel_new (window, data_dir);
	g_free (data_dir);

	xed_panel_add_item_with_stock_icon (side_panel,
					      priv->taglist_panel,
					      _("Tags"),
					      GTK_STOCK_ADD);
}

static void
xed_taglist_plugin_deactivate (PeasActivatable *activatable)
{
    XedTaglistPluginPrivate *priv;
    XedWindow *window;
	XedPanel *side_panel;

	xed_debug (DEBUG_PLUGINS);

	priv = XED_TAGLIST_PLUGIN (activatable)->priv;
    window = XED_WINDOW (priv->window);
	side_panel = xed_window_get_side_panel (window);

	xed_panel_remove_item (side_panel, priv->taglist_panel);
}

static void
xed_taglist_plugin_update_state (PeasActivatable *activatable)
{
	XedTaglistPluginPrivate *priv;
    XedWindow *window;
	XedView *view;

	xed_debug (DEBUG_PLUGINS);

	priv = XED_TAGLIST_PLUGIN (activatable)->priv;
    window = XED_WINDOW (priv->window);
	view = xed_window_get_active_view (window);

	gtk_widget_set_sensitive (priv->taglist_panel,
				  (view != NULL) &&
				  gtk_text_view_get_editable (GTK_TEXT_VIEW (view)));
}

static void
xed_taglist_plugin_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
    XedTaglistPlugin *plugin = XED_TAGLIST_PLUGIN (object);

    switch (prop_id)
    {
        case PROP_OBJECT:
            plugin->priv->window = GTK_WIDGET (g_value_dup_object (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
xed_taglist_plugin_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
    XedTaglistPlugin *plugin = XED_TAGLIST_PLUGIN (object);

    switch (prop_id)
    {
        case PROP_OBJECT:
            g_value_set_object (value, plugin->priv->window);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
xed_taglist_plugin_class_init (XedTaglistPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = xed_taglist_plugin_finalize;
    object_class->dispose = xed_taglist_plugin_dispose;
    object_class->set_property = xed_taglist_plugin_set_property;
    object_class->get_property = xed_taglist_plugin_get_property;

	g_object_class_override_property (object_class, PROP_OBJECT, "object");

	g_type_class_add_private (object_class, sizeof (XedTaglistPluginPrivate));
}

static void
xed_taglist_plugin_class_finalize (XedTaglistPluginClass *klass)
{
    /* dummy function - used by G_DEFINE_DYNAMIC_TYPE_EXTENDED */
}

static void
peas_activatable_iface_init (PeasActivatableInterface *iface)
{
    iface->activate = xed_taglist_plugin_activate;
    iface->deactivate = xed_taglist_plugin_deactivate;
    iface->update_state = xed_taglist_plugin_update_state;
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
    xed_taglist_plugin_register_type (G_TYPE_MODULE (module));

    peas_object_module_register_extension_type (module,
                                                PEAS_TYPE_ACTIVATABLE,
                                                XED_TYPE_TAGLIST_PLUGIN);
}
