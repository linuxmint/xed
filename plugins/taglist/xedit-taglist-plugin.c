/*
 * xedit-taglist-plugin.h
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
 * Modified by the xedit Team, 2002-2005. See the AUTHORS file for a 
 * list of people on the xedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "xedit-taglist-plugin.h"
#include "xedit-taglist-plugin-panel.h"
#include "xedit-taglist-plugin-parser.h"

#include <glib/gi18n-lib.h>
#include <gmodule.h>

#include <xedit/xedit-plugin.h>
#include <xedit/xedit-debug.h>

#define WINDOW_DATA_KEY "XeditTaglistPluginWindowData"

#define XEDIT_TAGLIST_PLUGIN_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), XEDIT_TYPE_TAGLIST_PLUGIN, XeditTaglistPluginPrivate))

struct _XeditTaglistPluginPrivate
{
	gpointer dummy;
};

XEDIT_PLUGIN_REGISTER_TYPE_WITH_CODE (XeditTaglistPlugin, xedit_taglist_plugin,
	xedit_taglist_plugin_panel_register_type (module);
)

static void
xedit_taglist_plugin_init (XeditTaglistPlugin *plugin)
{
	plugin->priv = XEDIT_TAGLIST_PLUGIN_GET_PRIVATE (plugin);

	xedit_debug_message (DEBUG_PLUGINS, "XeditTaglistPlugin initializing");
}

static void
xedit_taglist_plugin_finalize (GObject *object)
{
/*
	XeditTaglistPlugin *plugin = XEDIT_TAGLIST_PLUGIN (object);
*/
	xedit_debug_message (DEBUG_PLUGINS, "XeditTaglistPlugin finalizing");

	free_taglist ();
	
	G_OBJECT_CLASS (xedit_taglist_plugin_parent_class)->finalize (object);
}

static void
impl_activate (XeditPlugin *plugin,
	       XeditWindow *window)
{
	XeditPanel *side_panel;
	GtkWidget *taglist_panel;
	gchar *data_dir;
	
	xedit_debug (DEBUG_PLUGINS);
	
	g_return_if_fail (g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY) == NULL);
	
	side_panel = xedit_window_get_side_panel (window);
	
	data_dir = xedit_plugin_get_data_dir (plugin);
	taglist_panel = xedit_taglist_plugin_panel_new (window, data_dir);
	g_free (data_dir);
	
	xedit_panel_add_item_with_stock_icon (side_panel, 
					      taglist_panel, 
					      _("Tags"), 
					      GTK_STOCK_ADD);

	g_object_set_data (G_OBJECT (window), 
			   WINDOW_DATA_KEY,
			   taglist_panel);
}

static void
impl_deactivate	(XeditPlugin *plugin,
		 XeditWindow *window)
{
	XeditPanel *side_panel;
	gpointer data;
	
	xedit_debug (DEBUG_PLUGINS);
	
	data = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);
	
	side_panel = xedit_window_get_side_panel (window);

	xedit_panel_remove_item (side_panel, 
			      	 GTK_WIDGET (data));
			      
	g_object_set_data (G_OBJECT (window), 
			   WINDOW_DATA_KEY,
			   NULL);
}

static void
impl_update_ui	(XeditPlugin *plugin,
		 XeditWindow *window)
{
	gpointer data;
	XeditView *view;
	
	xedit_debug (DEBUG_PLUGINS);
	
	data = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);
	
	view = xedit_window_get_active_view (window);
	
	gtk_widget_set_sensitive (GTK_WIDGET (data),
				  (view != NULL) &&
				  gtk_text_view_get_editable (GTK_TEXT_VIEW (view)));
}

static void
xedit_taglist_plugin_class_init (XeditTaglistPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	XeditPluginClass *plugin_class = XEDIT_PLUGIN_CLASS (klass);

	object_class->finalize = xedit_taglist_plugin_finalize;

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
	plugin_class->update_ui = impl_update_ui;

	g_type_class_add_private (object_class, sizeof (XeditTaglistPluginPrivate));
}
