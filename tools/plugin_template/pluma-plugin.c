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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "##(PLUGIN_MODULE)-plugin.h"

#include <glib/gi18n-lib.h>
#include <pluma/pluma-debug.h>

#define WINDOW_DATA_KEY	"##(PLUGIN_ID.camel)PluginWindowData"

#define ##(PLUGIN_ID.upper)_PLUGIN_GET_PRIVATE(object)	(G_TYPE_INSTANCE_GET_PRIVATE ((object), TYPE_##(PLUGIN_ID.upper)_PLUGIN, ##(PLUGIN_ID.camel)PluginPrivate))

struct _##(PLUGIN_ID.camel)PluginPrivate
{
	gpointer dummy;
};

PLUMA_PLUGIN_REGISTER_TYPE (##(PLUGIN_ID.camel)Plugin, ##(PLUGIN_ID.lower)_plugin)

##ifdef WITH_MENU
/* UI string. See pluma-ui.xml for reference */
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

typedef struct
{
	GtkActionGroup *action_group;
	guint           ui_id;
} WindowData;
##endif

static void
##(PLUGIN_ID.lower)_plugin_init (##(PLUGIN_ID.camel)Plugin *plugin)
{
	plugin->priv = ##(PLUGIN_ID.upper)_PLUGIN_GET_PRIVATE (plugin);

	pluma_debug_message (DEBUG_PLUGINS,
			     "##(PLUGIN_ID.camel)Plugin initializing");
}

static void
##(PLUGIN_ID.lower)_plugin_finalize (GObject *object)
{
	pluma_debug_message (DEBUG_PLUGINS,
			     "##(PLUGIN_ID.camel)Plugin finalizing");

	G_OBJECT_CLASS (##(PLUGIN_ID.lower)_plugin_parent_class)->finalize (object);
}

##ifdef WITH_MENU
static void
free_window_data (WindowData *data)
{
	g_return_if_fail (data != NULL);

	g_object_unref (data->action_group);
	g_free (data);
}
##endif

static void
impl_activate (PlumaPlugin *plugin,
	       PlumaWindow *window)
{
##ifdef WITH_MENU
	GtkUIManager *manager;
	WindowData *data;
##endif	

	pluma_debug (DEBUG_PLUGINS);

##ifdef WITH_MENU
	data = g_new (WindowData, 1);
	manager = pluma_window_get_ui_manager (window);

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

	g_object_set_data_full (G_OBJECT (window), 
				WINDOW_DATA_KEY, 
				data,
				(GDestroyNotify) free_window_data);
##endif	
}

static void
impl_deactivate (PlumaPlugin *plugin,
		 PlumaWindow *window)
{
##ifdef WITH_MENU
	GtkUIManager *manager;
	WindowData *data;
##endif

	pluma_debug (DEBUG_PLUGINS);

##ifdef WITH_MENU
	manager = pluma_window_get_ui_manager (window);

	data = (WindowData *) g_object_get_data (G_OBJECT (window),
						 WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	gtk_ui_manager_remove_ui (manager, data->ui_id);
	gtk_ui_manager_remove_action_group (manager, data->action_group);

	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);
##endif
}

static void
impl_update_ui (PlumaPlugin *plugin,
		PlumaWindow *window)
{
	pluma_debug (DEBUG_PLUGINS);
}

##ifdef WITH_CONFIGURE_DIALOG
static GtkWidget *
impl_create_configure_dialog (PlumaPlugin *plugin)
{
	pluma_debug (DEBUG_PLUGINS);
}
##endif

static void
##(PLUGIN_ID.lower)_plugin_class_init (##(PLUGIN_ID.camel)PluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	PlumaPluginClass *plugin_class = PLUMA_PLUGIN_CLASS (klass);

	object_class->finalize = ##(PLUGIN_ID.lower)_plugin_finalize;

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
	plugin_class->update_ui = impl_update_ui;
##ifdef WITH_CONFIGURE_DIALOG
	plugin_class->create_configure_dialog = impl_create_configure_dialog;
##endif

	g_type_class_add_private (object_class, 
				  sizeof (##(PLUGIN_ID.camel)PluginPrivate));
}
