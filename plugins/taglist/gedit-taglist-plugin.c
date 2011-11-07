/*
 * gedit-taglist-plugin.h
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

/*
 * Modified by the gedit Team, 2002-2005. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gedit-taglist-plugin.h"
#include "gedit-taglist-plugin-panel.h"
#include "gedit-taglist-plugin-parser.h"

#include <glib/gi18n-lib.h>
#include <gmodule.h>

#include <gedit/gedit-plugin.h>
#include <gedit/gedit-debug.h>

#define WINDOW_DATA_KEY "GeditTaglistPluginWindowData"

#define GEDIT_TAGLIST_PLUGIN_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GEDIT_TYPE_TAGLIST_PLUGIN, GeditTaglistPluginPrivate))

struct _GeditTaglistPluginPrivate
{
	gpointer dummy;
};

GEDIT_PLUGIN_REGISTER_TYPE_WITH_CODE (GeditTaglistPlugin, gedit_taglist_plugin,
	gedit_taglist_plugin_panel_register_type (module);
)

static void
gedit_taglist_plugin_init (GeditTaglistPlugin *plugin)
{
	plugin->priv = GEDIT_TAGLIST_PLUGIN_GET_PRIVATE (plugin);

	gedit_debug_message (DEBUG_PLUGINS, "GeditTaglistPlugin initializing");
}

static void
gedit_taglist_plugin_finalize (GObject *object)
{
/*
	GeditTaglistPlugin *plugin = GEDIT_TAGLIST_PLUGIN (object);
*/
	gedit_debug_message (DEBUG_PLUGINS, "GeditTaglistPlugin finalizing");

	free_taglist ();
	
	G_OBJECT_CLASS (gedit_taglist_plugin_parent_class)->finalize (object);
}

static void
impl_activate (GeditPlugin *plugin,
	       GeditWindow *window)
{
	GeditPanel *side_panel;
	GtkWidget *taglist_panel;
	gchar *data_dir;
	
	gedit_debug (DEBUG_PLUGINS);
	
	g_return_if_fail (g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY) == NULL);
	
	side_panel = gedit_window_get_side_panel (window);
	
	data_dir = gedit_plugin_get_data_dir (plugin);
	taglist_panel = gedit_taglist_plugin_panel_new (window, data_dir);
	g_free (data_dir);
	
	gedit_panel_add_item_with_stock_icon (side_panel, 
					      taglist_panel, 
					      _("Tags"), 
					      GTK_STOCK_ADD);

	g_object_set_data (G_OBJECT (window), 
			   WINDOW_DATA_KEY,
			   taglist_panel);
}

static void
impl_deactivate	(GeditPlugin *plugin,
		 GeditWindow *window)
{
	GeditPanel *side_panel;
	gpointer data;
	
	gedit_debug (DEBUG_PLUGINS);
	
	data = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);
	
	side_panel = gedit_window_get_side_panel (window);

	gedit_panel_remove_item (side_panel, 
			      	 GTK_WIDGET (data));
			      
	g_object_set_data (G_OBJECT (window), 
			   WINDOW_DATA_KEY,
			   NULL);
}

static void
impl_update_ui	(GeditPlugin *plugin,
		 GeditWindow *window)
{
	gpointer data;
	GeditView *view;
	
	gedit_debug (DEBUG_PLUGINS);
	
	data = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);
	
	view = gedit_window_get_active_view (window);
	
	gtk_widget_set_sensitive (GTK_WIDGET (data),
				  (view != NULL) &&
				  gtk_text_view_get_editable (GTK_TEXT_VIEW (view)));
}

static void
gedit_taglist_plugin_class_init (GeditTaglistPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GeditPluginClass *plugin_class = GEDIT_PLUGIN_CLASS (klass);

	object_class->finalize = gedit_taglist_plugin_finalize;

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
	plugin_class->update_ui = impl_update_ui;

	g_type_class_add_private (object_class, sizeof (GeditTaglistPluginPrivate));
}
