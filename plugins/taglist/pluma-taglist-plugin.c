/*
 * pluma-taglist-plugin.h
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
 * Modified by the pluma Team, 2002-2005. See the AUTHORS file for a 
 * list of people on the pluma Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "pluma-taglist-plugin.h"
#include "pluma-taglist-plugin-panel.h"
#include "pluma-taglist-plugin-parser.h"

#include <glib/gi18n-lib.h>
#include <gmodule.h>

#include <pluma/pluma-plugin.h>
#include <pluma/pluma-debug.h>

#define WINDOW_DATA_KEY "PlumaTaglistPluginWindowData"

#define PLUMA_TAGLIST_PLUGIN_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), PLUMA_TYPE_TAGLIST_PLUGIN, PlumaTaglistPluginPrivate))

struct _PlumaTaglistPluginPrivate
{
	gpointer dummy;
};

PLUMA_PLUGIN_REGISTER_TYPE_WITH_CODE (PlumaTaglistPlugin, pluma_taglist_plugin,
	pluma_taglist_plugin_panel_register_type (module);
)

static void
pluma_taglist_plugin_init (PlumaTaglistPlugin *plugin)
{
	plugin->priv = PLUMA_TAGLIST_PLUGIN_GET_PRIVATE (plugin);

	pluma_debug_message (DEBUG_PLUGINS, "PlumaTaglistPlugin initializing");
}

static void
pluma_taglist_plugin_finalize (GObject *object)
{
/*
	PlumaTaglistPlugin *plugin = PLUMA_TAGLIST_PLUGIN (object);
*/
	pluma_debug_message (DEBUG_PLUGINS, "PlumaTaglistPlugin finalizing");

	free_taglist ();
	
	G_OBJECT_CLASS (pluma_taglist_plugin_parent_class)->finalize (object);
}

static void
impl_activate (PlumaPlugin *plugin,
	       PlumaWindow *window)
{
	PlumaPanel *side_panel;
	GtkWidget *taglist_panel;
	gchar *data_dir;
	
	pluma_debug (DEBUG_PLUGINS);
	
	g_return_if_fail (g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY) == NULL);
	
	side_panel = pluma_window_get_side_panel (window);
	
	data_dir = pluma_plugin_get_data_dir (plugin);
	taglist_panel = pluma_taglist_plugin_panel_new (window, data_dir);
	g_free (data_dir);
	
	pluma_panel_add_item_with_stock_icon (side_panel, 
					      taglist_panel, 
					      _("Tags"), 
					      GTK_STOCK_ADD);

	g_object_set_data (G_OBJECT (window), 
			   WINDOW_DATA_KEY,
			   taglist_panel);
}

static void
impl_deactivate	(PlumaPlugin *plugin,
		 PlumaWindow *window)
{
	PlumaPanel *side_panel;
	gpointer data;
	
	pluma_debug (DEBUG_PLUGINS);
	
	data = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);
	
	side_panel = pluma_window_get_side_panel (window);

	pluma_panel_remove_item (side_panel, 
			      	 GTK_WIDGET (data));
			      
	g_object_set_data (G_OBJECT (window), 
			   WINDOW_DATA_KEY,
			   NULL);
}

static void
impl_update_ui	(PlumaPlugin *plugin,
		 PlumaWindow *window)
{
	gpointer data;
	PlumaView *view;
	
	pluma_debug (DEBUG_PLUGINS);
	
	data = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);
	
	view = pluma_window_get_active_view (window);
	
	gtk_widget_set_sensitive (GTK_WIDGET (data),
				  (view != NULL) &&
				  gtk_text_view_get_editable (GTK_TEXT_VIEW (view)));
}

static void
pluma_taglist_plugin_class_init (PlumaTaglistPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	PlumaPluginClass *plugin_class = PLUMA_PLUGIN_CLASS (klass);

	object_class->finalize = pluma_taglist_plugin_finalize;

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
	plugin_class->update_ui = impl_update_ui;

	g_type_class_add_private (object_class, sizeof (PlumaTaglistPluginPrivate));
}
