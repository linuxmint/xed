/*
 * ##(FILENAME) - ##(DESCRIPTION)
 *
 * Copyright (C) ##(DATE_YEAR) - ##(AUTHOR_FULLNAME)
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include <glib/gi18n-lib.h>
#include <xed/xed-debug.h>

#include "##(PLUGIN_MODULE)-plugin.h"

struct _##(PLUGIN_ID.camel)PluginPrivate
{
    gpointer dummy;

##ifdef WITH_MENU
    GtkActionGroup *action_group;
    guint           ui_id;
##endif
};

struct _##(PLUGIN_ID.camel)Plugin
{
    PeasExtensionBase;
};

static void peas_activatable_iface_init (PeasActivatableInterface *iface);
static void peas_gtk_configurable_iface_init (PeasGtkConfigurableInterface *iface);

G_DEFINE_TYPE_WITH_PRIVATE (##(PLUGIN_ID.camel)Plugin, ##(PLUGIN_ID.lower)_plugin, PeasExtensionBase)

##ifdef WITH_MENU
/* UI string. See xed-ui.xml for reference */
static const gchar ui_str =
    "<ui>"
    "  <menubar name='MenuBar'>"
    "    <!-- Put your menu entries here -->"
    "  </menubar>"
    "</ui>";

/* UI actions */
static const GtkActionEntry action_entries[] =
    {
        /* Put your actions here */
    };

##endif

static void
##(PLUGIN_ID.lower)_plugin_init (##(PLUGIN_ID.camel)Plugin *plugin)
{
    plugin->priv = ##(PLUGIN_ID.upper)_PLUGIN_GET_PRIVATE (plugin);

    xed_debug_message (DEBUG_PLUGINS, "##(PLUGIN_ID.camel)Plugin initializing");
}

static void
##(PLUGIN_ID.lower)_plugin_finalize (GObject *object)
{
    xed_debug_message (DEBUG_PLUGINS, "##(PLUGIN_ID.camel)Plugin finalizing");

    G_OBJECT_CLASS (##(PLUGIN_ID.lower)_plugin_parent_class)->finalize (object);
}

static void
##(PLUGIN_ID.lower)_plugin_dispose (GObject *object)
{
    ##(PLUGIN_ID.camelPlugin *plugin = ##(PLUGIN_ID.upper)_PLUGIN (object);
    xed_debug_message (DEBUG_PLUGINS, "##(PLUGIN_ID.camel)Plugin disposing");
    if (plugin->priv->window != NULL)
    {
        g_object_unref (plugin->priv->window);
        plugin->priv->window = NULL;
    }
    if (plugin->priv->action_group)
    {
        g_object_unref (plugin->priv->action_group);
        plugin->priv->action_group = NULL;
    }

    G_OBJECT_CLASS (##(PLUGIN_ID.lower)_plugin_parent_class)->dispose (object);
}

static void
##(PLUGIN_ID.lower)_plugin_activate (PeasActivatable *activatable)
{
##ifdef WITH_MENU
    ##(PLUGIN_ID.camel)Plugin *plugin;
    ##(PLUGIN_ID.camel)PluginPrivate *data;
    XedWindow *window;
    GtkUIManager *manager;
##endif

    xed_debug (DEBUG_PLUGINS);

##ifdef WITH_MENU
    plugin = ##(PLUGIN_ID.upper)_PLUGIN (activatable);
    data = plugin->priv;
    window = XED_WINDOW (data->window);

    manager = xed_window_get_ui_manager (window);

    data->action_group = gtk_action_group_new ("##(PLUGIN_ID.camel)PluginActions");
    gtk_action_group_set_translation_domain (data->action_group,
                                             GETTEXT_PACKAGE);
    gtk_action_group_add_actions (data->action_group,
                                  action_entries,
                                  G_N_ELEMENTS (action_entries),
                                  window);

    gtk_ui_manager_insert_action_group (manager, data->action_group, -1);

    data->ui_id = gtk_ui_manager_add_ui_from_string (manager, ui_str,
                                                     -1, NULL);
##endif
}

static void
##(PLUGIN_ID.lower)_plugin_deactivate (PeasActivatable *activatable)
{
##ifdef WITH_MENU
    ##(PLUGIN_ID.camel)PluginPrivate *data;
    XedWindow *window;
    GtkUIManager *manager;
##endif

    xed_debug (DEBUG_PLUGINS);

##ifdef WITH_MENU
    data = ##(PLUGIN_ID.upper)_PLUGIN (activatable)->priv;
    window = XED_WINDOW (data->window);

    manager = xed_window_get_ui_manager (window);

    data = (WindowData *) g_object_get_data (G_OBJECT (window),
                         WINDOW_DATA_KEY);
    g_return_if_fail (data != NULL);

    gtk_ui_manager_remove_ui (manager, data->ui_id);
    gtk_ui_manager_remove_action_group (manager, data->action_group);
##endif
}

static void
##(PLUGIN_ID.lower)_plugin_update_state (PeasActivatable *activatable)
{
    xed_debug (DEBUG_PLUGINS);
}

static void
peas_activatable_iface_init (PeasActivatableInterface *iface)
{
    iface->activate = ##(PLUGIN_ID.lower)_plugin_activate;
    iface->deactivate = ##(PLUGIN_ID.lower)_plugin_deactivate;
    iface->update_state = ##(PLUGIN_ID.lower)_plugin_update_state;
}

##ifdef WITH_CONFIGURE_DIALOG
static void
peas_gtk_configurable_iface_init (PeasGtkConfigurableInterface *iface)
{
    iface->create_configure_widget = xed_time_plugin_create_configure_widget;
}
##endif

static void
##(PLUGIN_ID.lower)_plugin_class_init (##(PLUGIN_ID.camel)PluginClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    XedPluginClass *plugin_class = XED_PLUGIN_CLASS (klass);

    object_class->finalize = ##(PLUGIN_ID.lower)_plugin_finalize;
    object_class->dispose = ##(PLUGIN_ID.lower)_plugin_dispose;
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
    ##(PLUGIN_ID.lower)_plugin_register_type (G_TYPE_MODULE (module));
    peas_object_module_register_extension_type (module,
                                                PEAS_TYPE_ACTIVATABLE,
                                                ##(PLUGIN_ID.lower)_TYPE_PLUGIN);

    peas_object_module_register_extension_type (module,
                                                PEAS_GTK_TYPE_CONFIGURABLE,
                                                ##(PLUGIN_ID.lower)_TYPE_PLUGIN);
}
