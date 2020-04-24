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

#include <config.h>
#include <glib/gi18n-lib.h>
#include <xed/xed-window.h>
#include <xed/xed-window-activatable.h>
#include <xed/xed-debug.h>

#include "xed-taglist-plugin.h"
#include "xed-taglist-plugin-panel.h"
#include "xed-taglist-plugin-parser.h"

#define XED_TAGLIST_PLUGIN_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), XED_TYPE_TAGLIST_PLUGIN, XedTaglistPluginPrivate))

struct _XedTaglistPluginPrivate
{
    XedWindow *window;

    GtkWidget *taglist_panel;
};

static void xed_window_activatable_iface_init (XedWindowActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (XedTaglistPlugin,
                                xed_taglist_plugin,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (XED_TYPE_WINDOW_ACTIVATABLE,
                                                               xed_window_activatable_iface_init) \
                                                                                                  \
                                _xed_taglist_plugin_panel_register_type (type_module);            \
)

enum
{
    PROP_0,
    PROP_WINDOW
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

    g_clear_object (&plugin->priv->window);

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
xed_taglist_plugin_activate (XedWindowActivatable *activatable)
{
    XedTaglistPluginPrivate *priv;
    XedPanel *side_panel;
    gchar *data_dir;

    xed_debug (DEBUG_PLUGINS);

    priv = XED_TAGLIST_PLUGIN (activatable)->priv;
    side_panel = xed_window_get_side_panel (priv->window);

    data_dir = peas_extension_base_get_data_dir (PEAS_EXTENSION_BASE (activatable));
    priv->taglist_panel = xed_taglist_plugin_panel_new (priv->window, data_dir);
    g_free (data_dir);

    xed_panel_add_item (side_panel, priv->taglist_panel, _("Tags"), "list-add-symbolic");
}

static void
xed_taglist_plugin_deactivate (XedWindowActivatable *activatable)
{
    XedTaglistPluginPrivate *priv;
    XedPanel *side_panel;

    xed_debug (DEBUG_PLUGINS);

    priv = XED_TAGLIST_PLUGIN (activatable)->priv;
    side_panel = xed_window_get_side_panel (priv->window);

    xed_panel_remove_item (side_panel, priv->taglist_panel);
}

static void
xed_taglist_plugin_update_state (XedWindowActivatable *activatable)
{
    XedTaglistPluginPrivate *priv;
    XedView *view;

    xed_debug (DEBUG_PLUGINS);

    priv = XED_TAGLIST_PLUGIN (activatable)->priv;
    view = xed_window_get_active_view (priv->window);

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
        case PROP_WINDOW:
            plugin->priv->window = XED_WINDOW (g_value_dup_object (value));
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
        case PROP_WINDOW:
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

    g_object_class_override_property (object_class, PROP_WINDOW, "window");

    g_type_class_add_private (object_class, sizeof (XedTaglistPluginPrivate));
}

static void
xed_taglist_plugin_class_finalize (XedTaglistPluginClass *klass)
{
    /* dummy function - used by G_DEFINE_DYNAMIC_TYPE_EXTENDED */
}

static void
xed_window_activatable_iface_init (XedWindowActivatableInterface *iface)
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
                                                XED_TYPE_WINDOW_ACTIVATABLE,
                                                XED_TYPE_TAGLIST_PLUGIN);
}
