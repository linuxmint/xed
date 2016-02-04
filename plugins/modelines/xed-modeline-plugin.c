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
#	include <config.h>
#endif

#include <glib/gi18n-lib.h>
#include <gmodule.h>
#include "xed-modeline-plugin.h"
#include "modeline-parser.h"

#include <xed/xed-debug.h>
#include <xed/xed-utils.h>

#define WINDOW_DATA_KEY "XedModelinePluginWindowData"
#define DOCUMENT_DATA_KEY "XedModelinePluginDocumentData"

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

static void	xed_modeline_plugin_activate (XedPlugin *plugin, XedWindow *window);
static void	xed_modeline_plugin_deactivate (XedPlugin *plugin, XedWindow *window);
static GObject	*xed_modeline_plugin_constructor (GType type, guint n_construct_properties, GObjectConstructParam *construct_param);
static void	xed_modeline_plugin_finalize (GObject *object);

XED_PLUGIN_REGISTER_TYPE(XedModelinePlugin, xed_modeline_plugin)

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
xed_modeline_plugin_class_init (XedModelinePluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	XedPluginClass *plugin_class = XED_PLUGIN_CLASS (klass);

	object_class->constructor = xed_modeline_plugin_constructor;
	object_class->finalize = xed_modeline_plugin_finalize;

	plugin_class->activate = xed_modeline_plugin_activate;
	plugin_class->deactivate = xed_modeline_plugin_deactivate;
}

static GObject *
xed_modeline_plugin_constructor (GType                  type,
				   guint                  n_construct_properties,
				   GObjectConstructParam *construct_param)
{
	GObject *object;
	gchar *data_dir;

	object = G_OBJECT_CLASS (xed_modeline_plugin_parent_class)->constructor (type,
										   n_construct_properties,
										   construct_param);

	data_dir = xed_plugin_get_data_dir (XED_PLUGIN (object));

	modeline_parser_init (data_dir);

	g_free (data_dir);

	return object;
}

static void
xed_modeline_plugin_init (XedModelinePlugin *plugin)
{
	xed_debug_message (DEBUG_PLUGINS, "XedModelinePlugin initializing");
}

static void
xed_modeline_plugin_finalize (GObject *object)
{
	xed_debug_message (DEBUG_PLUGINS, "XedModelinePlugin finalizing");

	modeline_parser_shutdown ();

	G_OBJECT_CLASS (xed_modeline_plugin_parent_class)->finalize (object);
}

static void
on_document_loaded_or_saved (XedDocument *document,
			     const GError  *error,
			     GtkSourceView *view)
{
	modeline_parser_apply_modeline (view);
}

static void
connect_handlers (XedView *view)
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
disconnect_handlers (XedView *view)
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
on_window_tab_added (XedWindow *window,
		     XedTab *tab,
		     gpointer user_data)
{
	connect_handlers (xed_tab_get_view (tab));
}

static void
on_window_tab_removed (XedWindow *window,
		       XedTab *tab,
		       gpointer user_data)
{
	disconnect_handlers (xed_tab_get_view (tab));
}

static void
xed_modeline_plugin_activate (XedPlugin *plugin,
				XedWindow *window)
{
	WindowData *wdata;
	GList *views;
	GList *l;

	xed_debug (DEBUG_PLUGINS);

	views = xed_window_get_views (window);
	for (l = views; l != NULL; l = l->next)
	{
		connect_handlers (XED_VIEW (l->data));
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
xed_modeline_plugin_deactivate (XedPlugin *plugin,
				  XedWindow *window)
{
	WindowData *wdata;
	GList *views;
	GList *l;

	xed_debug (DEBUG_PLUGINS);

	wdata = g_object_steal_data (G_OBJECT (window), WINDOW_DATA_KEY);

	g_signal_handler_disconnect (window, wdata->tab_added_handler_id);
	g_signal_handler_disconnect (window, wdata->tab_removed_handler_id);

	window_data_free (wdata);

	views = xed_window_get_views (window);

	for (l = views; l != NULL; l = l->next)
	{
		disconnect_handlers (XED_VIEW (l->data));
		
		modeline_parser_deactivate (GTK_SOURCE_VIEW (l->data));
	}
	
	g_list_free (views);
}

