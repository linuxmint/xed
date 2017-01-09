/*
 * xed-modeline-plugin.c
 * Emacs, Kate and Vim-style modelines support for xed.
 *
 * Copyright (C) 2005-2007 - Steve Frécinaux <code@istique.net>
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
 */

#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif

#include <glib/gi18n-lib.h>
#include <gmodule.h>
#include "xed-modeline-plugin.h"
#include "modeline-parser.h"

#include <xed/xed-debug.h>
#include <xed/xed-view-activatable.h>
#include <xed/xed-view.h>

struct _XedModelinePluginPrivate
{
    XedView *view;

    gulong document_loaded_handler_id;
    gulong document_saved_handler_id;
};

enum
{
    PROP_0,
    PROP_VIEW
};

static void xed_view_activatable_iface_init (XedViewActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (XedModelinePlugin,
                                xed_modeline_plugin,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (XED_TYPE_VIEW_ACTIVATABLE,
                                                               xed_view_activatable_iface_init))

static void
xed_modeline_plugin_constructed (GObject *object)
{
    gchar *data_dir;

    data_dir = peas_extension_base_get_data_dir (PEAS_EXTENSION_BASE (object));

    modeline_parser_init (data_dir);

    g_free (data_dir);

    G_OBJECT_CLASS (xed_modeline_plugin_parent_class)->constructed (object);
}

static void
xed_modeline_plugin_init (XedModelinePlugin *plugin)
{
    xed_debug_message (DEBUG_PLUGINS, "XedModelinePlugin initializing");

    plugin->priv = G_TYPE_INSTANCE_GET_PRIVATE (plugin,
                                                XED_TYPE_MODELINE_PLUGIN,
                                                XedModelinePluginPrivate);
}

static void
xed_modeline_plugin_dispose (GObject *object)
{
    XedModelinePlugin *plugin = XED_MODELINE_PLUGIN (object);

    xed_debug_message (DEBUG_PLUGINS, "XedModelinePlugin disposing");

    g_clear_object (&plugin->priv->view);

    G_OBJECT_CLASS (xed_modeline_plugin_parent_class)->dispose (object);
}

static void
xed_modeline_plugin_finalize (GObject *object)
{
    xed_debug_message (DEBUG_PLUGINS, "XedModelinePlugin finalizing");

    modeline_parser_shutdown ();

    G_OBJECT_CLASS (xed_modeline_plugin_parent_class)->finalize (object);
}

static void
xed_modeline_plugin_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
    XedModelinePlugin *plugin = XED_MODELINE_PLUGIN (object);

    switch (prop_id)
    {
        case PROP_VIEW:
            plugin->priv->view = XED_VIEW (g_value_dup_object (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
xed_modeline_plugin_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
    XedModelinePlugin *plugin = XED_MODELINE_PLUGIN (object);

    switch (prop_id)
    {
        case PROP_VIEW:
            g_value_set_object (value, plugin->priv->view);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
on_document_loaded_or_saved (XedDocument   *document,
                             const GError  *error,
                             GtkSourceView *view)
{
    modeline_parser_apply_modeline (view);
}

static void
xed_modeline_plugin_activate (XedViewActivatable *activatable)
{
    XedModelinePlugin *plugin;
    GtkTextBuffer *doc;

    xed_debug (DEBUG_PLUGINS);

    plugin = XED_MODELINE_PLUGIN (activatable);

    modeline_parser_apply_modeline (GTK_SOURCE_VIEW (plugin->priv->view));

    doc = gtk_text_view_get_buffer (GTK_TEXT_VIEW (plugin->priv->view));

    plugin->priv->document_loaded_handler_id =
        g_signal_connect (doc, "loaded",
                          G_CALLBACK (on_document_loaded_or_saved), plugin->priv->view);
    plugin->priv->document_saved_handler_id =
        g_signal_connect (doc, "saved",
                          G_CALLBACK (on_document_loaded_or_saved), plugin->priv->view);
}

static void
xed_modeline_plugin_deactivate (XedViewActivatable *activatable)
{
    XedModelinePlugin *plugin;
    GtkTextBuffer *doc;

    xed_debug (DEBUG_PLUGINS);

    plugin = XED_MODELINE_PLUGIN (activatable);

    doc = gtk_text_view_get_buffer (GTK_TEXT_VIEW (plugin->priv->view));

    g_signal_handler_disconnect (doc, plugin->priv->document_loaded_handler_id);
    g_signal_handler_disconnect (doc, plugin->priv->document_saved_handler_id);
}

static void
xed_modeline_plugin_class_init (XedModelinePluginClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->constructed = xed_modeline_plugin_constructed;
    object_class->dispose = xed_modeline_plugin_dispose;
    object_class->finalize = xed_modeline_plugin_finalize;
    object_class->set_property = xed_modeline_plugin_set_property;
    object_class->get_property = xed_modeline_plugin_get_property;

    g_object_class_override_property (object_class, PROP_VIEW, "view");

    g_type_class_add_private (klass, sizeof (XedModelinePluginPrivate));
}

static void
xed_view_activatable_iface_init (XedViewActivatableInterface *iface)
{
    iface->activate = xed_modeline_plugin_activate;
    iface->deactivate = xed_modeline_plugin_deactivate;
}

static void
xed_modeline_plugin_class_finalize (XedModelinePluginClass *klass)
{
   /* dummy function - used by G_DEFINE_DYNAMIC_TYPE_EXTENDED */
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
   xed_modeline_plugin_register_type (G_TYPE_MODULE (module));

   peas_object_module_register_extension_type (module,
                                               XED_TYPE_VIEW_ACTIVATABLE,
                                               XED_TYPE_MODELINE_PLUGIN);
}
