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

#include <xed/xed-plugin.h>
#include <xed/xed-debug.h>

#define WINDOW_DATA_KEY "XedTaglistPluginWindowData"

#define XED_TAGLIST_PLUGIN_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), XED_TYPE_TAGLIST_PLUGIN, XedTaglistPluginPrivate))

struct _XedTaglistPluginPrivate
{
	gpointer dummy;
};

XED_PLUGIN_REGISTER_TYPE_WITH_CODE (XedTaglistPlugin, xed_taglist_plugin,
	xed_taglist_plugin_panel_register_type (module);
)

static void
xed_taglist_plugin_init (XedTaglistPlugin *plugin)
{
	plugin->priv = XED_TAGLIST_PLUGIN_GET_PRIVATE (plugin);

	xed_debug_message (DEBUG_PLUGINS, "XedTaglistPlugin initializing");
}

static void
xed_taglist_plugin_finalize (GObject *object)
{
/*
	XedTaglistPlugin *plugin = XED_TAGLIST_PLUGIN (object);
*/
	xed_debug_message (DEBUG_PLUGINS, "XedTaglistPlugin finalizing");

	free_taglist ();
	
	G_OBJECT_CLASS (xed_taglist_plugin_parent_class)->finalize (object);
}

static void
impl_activate (XedPlugin *plugin,
	       XedWindow *window)
{
	XedPanel *side_panel;
	GtkWidget *taglist_panel;
	gchar *data_dir;
	
	xed_debug (DEBUG_PLUGINS);
	
	g_return_if_fail (g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY) == NULL);
	
	side_panel = xed_window_get_side_panel (window);
	
	data_dir = xed_plugin_get_data_dir (plugin);
	taglist_panel = xed_taglist_plugin_panel_new (window, data_dir);
	g_free (data_dir);
	
	xed_panel_add_item_with_stock_icon (side_panel, 
					      taglist_panel, 
					      _("Tags"), 
					      GTK_STOCK_ADD);

	g_object_set_data (G_OBJECT (window), 
			   WINDOW_DATA_KEY,
			   taglist_panel);
}

static void
impl_deactivate	(XedPlugin *plugin,
		 XedWindow *window)
{
	XedPanel *side_panel;
	gpointer data;
	
	xed_debug (DEBUG_PLUGINS);
	
	data = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);
	
	side_panel = xed_window_get_side_panel (window);

	xed_panel_remove_item (side_panel, 
			      	 GTK_WIDGET (data));
			      
	g_object_set_data (G_OBJECT (window), 
			   WINDOW_DATA_KEY,
			   NULL);
}

static void
impl_update_ui	(XedPlugin *plugin,
		 XedWindow *window)
{
	gpointer data;
	XedView *view;
	
	xed_debug (DEBUG_PLUGINS);
	
	data = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);
	
	view = xed_window_get_active_view (window);
	
	gtk_widget_set_sensitive (GTK_WIDGET (data),
				  (view != NULL) &&
				  gtk_text_view_get_editable (GTK_TEXT_VIEW (view)));
}

static void
xed_taglist_plugin_class_init (XedTaglistPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	XedPluginClass *plugin_class = XED_PLUGIN_CLASS (klass);

	object_class->finalize = xed_taglist_plugin_finalize;

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
	plugin_class->update_ui = impl_update_ui;

	g_type_class_add_private (object_class, sizeof (XedTaglistPluginPrivate));
}
