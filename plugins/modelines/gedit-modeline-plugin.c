/*
 * gedit-modeline-plugin.c
 * Emacs, Kate and Vim-style modelines support for gedit.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <glib/gi18n-lib.h>
#include <gmodule.h>
#include "gedit-modeline-plugin.h"
#include "modeline-parser.h"

#include <gedit/gedit-debug.h>
#include <gedit/gedit-utils.h>

#define WINDOW_DATA_KEY "GeditModelinePluginWindowData"
#define DOCUMENT_DATA_KEY "GeditModelinePluginDocumentData"

typedef struct
{
	gulong tab_added_handler_id;
	gulong tab_removed_handler_id;
} WindowData;

typedef struct
{
	gulong document_loaded_handler_id;
	gulong document_saved_handler_id;
} DocumentData;

static void	gedit_modeline_plugin_activate (GeditPlugin *plugin, GeditWindow *window);
static void	gedit_modeline_plugin_deactivate (GeditPlugin *plugin, GeditWindow *window);
static GObject	*gedit_modeline_plugin_constructor (GType type, guint n_construct_properties, GObjectConstructParam *construct_param);
static void	gedit_modeline_plugin_finalize (GObject *object);

GEDIT_PLUGIN_REGISTER_TYPE(GeditModelinePlugin, gedit_modeline_plugin)

static void
window_data_free (WindowData *wdata)
{
	g_slice_free (WindowData, wdata);
}

static void
document_data_free (DocumentData *ddata)
{
	g_slice_free (DocumentData, ddata);
}

static void
gedit_modeline_plugin_class_init (GeditModelinePluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GeditPluginClass *plugin_class = GEDIT_PLUGIN_CLASS (klass);

	object_class->constructor = gedit_modeline_plugin_constructor;
	object_class->finalize = gedit_modeline_plugin_finalize;

	plugin_class->activate = gedit_modeline_plugin_activate;
	plugin_class->deactivate = gedit_modeline_plugin_deactivate;
}

static GObject *
gedit_modeline_plugin_constructor (GType                  type,
				   guint                  n_construct_properties,
				   GObjectConstructParam *construct_param)
{
	GObject *object;
	gchar *data_dir;

	object = G_OBJECT_CLASS (gedit_modeline_plugin_parent_class)->constructor (type,
										   n_construct_properties,
										   construct_param);

	data_dir = gedit_plugin_get_data_dir (GEDIT_PLUGIN (object));

	modeline_parser_init (data_dir);

	g_free (data_dir);

	return object;
}

static void
gedit_modeline_plugin_init (GeditModelinePlugin *plugin)
{
	gedit_debug_message (DEBUG_PLUGINS, "GeditModelinePlugin initializing");
}

static void
gedit_modeline_plugin_finalize (GObject *object)
{
	gedit_debug_message (DEBUG_PLUGINS, "GeditModelinePlugin finalizing");

	modeline_parser_shutdown ();

	G_OBJECT_CLASS (gedit_modeline_plugin_parent_class)->finalize (object);
}

static void
on_document_loaded_or_saved (GeditDocument *document,
			     const GError  *error,
			     GtkSourceView *view)
{
	modeline_parser_apply_modeline (view);
}

static void
connect_handlers (GeditView *view)
{
	DocumentData *data;
        GtkTextBuffer *doc;

        doc = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

        data = g_slice_new (DocumentData);

	data->document_loaded_handler_id =
		g_signal_connect (doc, "loaded",
				  G_CALLBACK (on_document_loaded_or_saved),
				  view);
	data->document_saved_handler_id =
		g_signal_connect (doc, "saved",
				  G_CALLBACK (on_document_loaded_or_saved),
				  view);

	g_object_set_data_full (G_OBJECT (doc), DOCUMENT_DATA_KEY,
				data, (GDestroyNotify) document_data_free);
}

static void
disconnect_handlers (GeditView *view)
{
	DocumentData *data;
	GtkTextBuffer *doc;

	doc = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

	data = g_object_steal_data (G_OBJECT (doc), DOCUMENT_DATA_KEY);

	if (data)
	{
		g_signal_handler_disconnect (doc, data->document_loaded_handler_id);
		g_signal_handler_disconnect (doc, data->document_saved_handler_id);

		document_data_free (data);
	}
	else
	{
		g_warning ("Modeline handlers not found");
	}
}

static void
on_window_tab_added (GeditWindow *window,
		     GeditTab *tab,
		     gpointer user_data)
{
	connect_handlers (gedit_tab_get_view (tab));
}

static void
on_window_tab_removed (GeditWindow *window,
		       GeditTab *tab,
		       gpointer user_data)
{
	disconnect_handlers (gedit_tab_get_view (tab));
}

static void
gedit_modeline_plugin_activate (GeditPlugin *plugin,
				GeditWindow *window)
{
	WindowData *wdata;
	GList *views;
	GList *l;

	gedit_debug (DEBUG_PLUGINS);

	views = gedit_window_get_views (window);
	for (l = views; l != NULL; l = l->next)
	{
		connect_handlers (GEDIT_VIEW (l->data));
		modeline_parser_apply_modeline (GTK_SOURCE_VIEW (l->data));
	}
	g_list_free (views);

	wdata = g_slice_new (WindowData);

	wdata->tab_added_handler_id =
		g_signal_connect (window, "tab-added",
				  G_CALLBACK (on_window_tab_added), NULL);

	wdata->tab_removed_handler_id =
		g_signal_connect (window, "tab-removed",
				  G_CALLBACK (on_window_tab_removed), NULL);

	g_object_set_data_full (G_OBJECT (window), WINDOW_DATA_KEY,
				wdata, (GDestroyNotify) window_data_free);
}

static void
gedit_modeline_plugin_deactivate (GeditPlugin *plugin,
				  GeditWindow *window)
{
	WindowData *wdata;
	GList *views;
	GList *l;

	gedit_debug (DEBUG_PLUGINS);

	wdata = g_object_steal_data (G_OBJECT (window), WINDOW_DATA_KEY);

	g_signal_handler_disconnect (window, wdata->tab_added_handler_id);
	g_signal_handler_disconnect (window, wdata->tab_removed_handler_id);

	window_data_free (wdata);

	views = gedit_window_get_views (window);

	for (l = views; l != NULL; l = l->next)
	{
		disconnect_handlers (GEDIT_VIEW (l->data));
		
		modeline_parser_deactivate (GTK_SOURCE_VIEW (l->data));
	}
	
	g_list_free (views);
}

