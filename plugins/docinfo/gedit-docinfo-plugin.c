/*
 * gedit-docinfo-plugin.c
 * 
 * Copyright (C) 2002-2005 Paolo Maggi 
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
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gedit-docinfo-plugin.h"

#include <string.h> /* For strlen (...) */

#include <glib/gi18n-lib.h>
#include <pango/pango-break.h>
#include <gmodule.h>

#include <gedit/gedit-debug.h>
#include <gedit/gedit-utils.h>

#define WINDOW_DATA_KEY "GeditDocInfoWindowData"
#define MENU_PATH "/MenuBar/ToolsMenu/ToolsOps_2"

GEDIT_PLUGIN_REGISTER_TYPE(GeditDocInfoPlugin, gedit_docinfo_plugin)

typedef struct
{
	GtkWidget *dialog;
	GtkWidget *file_name_label;
	GtkWidget *lines_label;
	GtkWidget *words_label;
	GtkWidget *chars_label;
	GtkWidget *chars_ns_label;
	GtkWidget *bytes_label;
	GtkWidget *selection_vbox;
	GtkWidget *selected_lines_label;
	GtkWidget *selected_words_label;
	GtkWidget *selected_chars_label;
	GtkWidget *selected_chars_ns_label;
	GtkWidget *selected_bytes_label;
} DocInfoDialog;

typedef struct
{
	GeditPlugin *plugin;

	GtkActionGroup *ui_action_group;
	guint ui_id;

	DocInfoDialog *dialog;
} WindowData;

static void docinfo_dialog_response_cb (GtkDialog   *widget,
					gint	    res_id,
					GeditWindow *window);

static void
docinfo_dialog_destroy_cb (GtkObject  *obj,
			   WindowData *data)
{
	gedit_debug (DEBUG_PLUGINS);

	if (data != NULL)
	{
		g_free (data->dialog);
		data->dialog = NULL;
	}
}

static DocInfoDialog *
get_docinfo_dialog (GeditWindow *window,
		    WindowData	*data)
{
	DocInfoDialog *dialog;
	gchar *data_dir;
	gchar *ui_file;
	GtkWidget *content;
	GtkWidget *error_widget;
	gboolean ret;

	gedit_debug (DEBUG_PLUGINS);

	dialog = g_new (DocInfoDialog, 1);

	data_dir = gedit_plugin_get_data_dir (data->plugin);
	ui_file = g_build_filename (data_dir, "docinfo.ui", NULL);
	ret = gedit_utils_get_ui_objects (ui_file,
					  NULL,
					  &error_widget,
					  "dialog", &dialog->dialog,
					  "docinfo_dialog_content", &content,
					  "file_name_label", &dialog->file_name_label,
					  "words_label", &dialog->words_label,
					  "bytes_label", &dialog->bytes_label,
					  "lines_label", &dialog->lines_label,
					  "chars_label", &dialog->chars_label,
					  "chars_ns_label", &dialog->chars_ns_label,
					  "selection_vbox", &dialog->selection_vbox,
					  "selected_words_label", &dialog->selected_words_label,
					  "selected_bytes_label", &dialog->selected_bytes_label,
					  "selected_lines_label", &dialog->selected_lines_label,
					  "selected_chars_label", &dialog->selected_chars_label,
					  "selected_chars_ns_label", &dialog->selected_chars_ns_label,
					  NULL);

	g_free (data_dir);
	g_free (ui_file);

	if (!ret)
	{
		const gchar *err_message;

		err_message = gtk_label_get_label (GTK_LABEL (error_widget));
		gedit_warning (GTK_WINDOW (window), "%s", err_message);

		g_free (dialog);
		gtk_widget_destroy (error_widget);

		return NULL;
	}

	gtk_dialog_set_default_response (GTK_DIALOG (dialog->dialog),
					 GTK_RESPONSE_OK);
	gtk_window_set_transient_for (GTK_WINDOW (dialog->dialog),
				      GTK_WINDOW (window));
	
	g_signal_connect (dialog->dialog,
			  "destroy",
			  G_CALLBACK (docinfo_dialog_destroy_cb),
			  data);
	g_signal_connect (dialog->dialog,
			  "response",
			  G_CALLBACK (docinfo_dialog_response_cb),
			  window);

	return dialog;
}

static void 
calculate_info (GeditDocument *doc,
		GtkTextIter   *start,
		GtkTextIter   *end, 
		gint          *chars,
		gint          *words,
		gint          *white_chars,
		gint          *bytes)
{	
	gchar *text;

	gedit_debug (DEBUG_PLUGINS);

	text = gtk_text_buffer_get_slice (GTK_TEXT_BUFFER (doc),
					  start,
					  end,
					  TRUE);

	*chars = g_utf8_strlen (text, -1);
	*bytes = strlen (text);

	if (*chars > 0)
	{
		PangoLogAttr *attrs;
		gint i;

		attrs = g_new0 (PangoLogAttr, *chars + 1);

		pango_get_log_attrs (text,
				     -1,
				     0,
				     pango_language_from_string ("C"),
				     attrs,
				     *chars + 1);

		for (i = 0; i < (*chars); i++)
		{
			if (attrs[i].is_white)
				++(*white_chars);

			if (attrs[i].is_word_start)
				++(*words);
		}

		g_free (attrs);
	}
	else
	{
		*white_chars = 0;
		*words = 0;
	}

	g_free (text);
}

static void
docinfo_real (GeditDocument *doc,
	      DocInfoDialog *dialog)
{
	GtkTextIter start, end;
	gint words = 0;
	gint chars = 0;
	gint white_chars = 0;
	gint lines = 0;
	gint bytes = 0;
	gchar *tmp_str;
	gchar *doc_name;

	gedit_debug (DEBUG_PLUGINS);

	gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (doc),
				    &start,
				    &end);

	lines = gtk_text_buffer_get_line_count (GTK_TEXT_BUFFER (doc));

	calculate_info (doc,
			&start, &end,
			&chars, &words, &white_chars, &bytes);

	if (chars == 0)
		lines = 0;

	gedit_debug_message (DEBUG_PLUGINS, "Chars: %d", chars);
	gedit_debug_message (DEBUG_PLUGINS, "Lines: %d", lines);
	gedit_debug_message (DEBUG_PLUGINS, "Words: %d", words);
	gedit_debug_message (DEBUG_PLUGINS, "Chars non-space: %d", chars - white_chars);
	gedit_debug_message (DEBUG_PLUGINS, "Bytes: %d", bytes);

	doc_name = gedit_document_get_short_name_for_display (doc);
	tmp_str = g_strdup_printf ("<span weight=\"bold\">%s</span>", doc_name);
	gtk_label_set_markup (GTK_LABEL (dialog->file_name_label), tmp_str);
	g_free (doc_name);
	g_free (tmp_str);

	tmp_str = g_strdup_printf("%d", lines);
	gtk_label_set_text (GTK_LABEL (dialog->lines_label), tmp_str);
	g_free (tmp_str);

	tmp_str = g_strdup_printf("%d", words);
	gtk_label_set_text (GTK_LABEL (dialog->words_label), tmp_str);
	g_free (tmp_str);

	tmp_str = g_strdup_printf("%d", chars);
	gtk_label_set_text (GTK_LABEL (dialog->chars_label), tmp_str);
	g_free (tmp_str);

	tmp_str = g_strdup_printf("%d", chars - white_chars);
	gtk_label_set_text (GTK_LABEL (dialog->chars_ns_label), tmp_str);
	g_free (tmp_str);

	tmp_str = g_strdup_printf("%d", bytes);
	gtk_label_set_text (GTK_LABEL (dialog->bytes_label), tmp_str);
	g_free (tmp_str);
}

static void
selectioninfo_real (GeditDocument *doc,
		    DocInfoDialog *dialog)
{
	gboolean sel;
	GtkTextIter start, end;
	gint words = 0;
	gint chars = 0;
	gint white_chars = 0;
	gint lines = 0;
	gint bytes = 0;
	gchar *tmp_str;

	gedit_debug (DEBUG_PLUGINS);

	sel = gtk_text_buffer_get_selection_bounds (GTK_TEXT_BUFFER (doc),
						    &start,
						    &end);

	if (sel)
	{
		lines = gtk_text_iter_get_line (&end) - gtk_text_iter_get_line (&start) + 1;
	
		calculate_info (doc,
				&start, &end,
				&chars, &words, &white_chars, &bytes);

		gedit_debug_message (DEBUG_PLUGINS, "Selected chars: %d", chars);
		gedit_debug_message (DEBUG_PLUGINS, "Selected lines: %d", lines);
		gedit_debug_message (DEBUG_PLUGINS, "Selected words: %d", words);
		gedit_debug_message (DEBUG_PLUGINS, "Selected chars non-space: %d", chars - white_chars);
		gedit_debug_message (DEBUG_PLUGINS, "Selected bytes: %d", bytes);

		gtk_widget_set_sensitive (dialog->selection_vbox, TRUE);
	}
	else
	{
		gtk_widget_set_sensitive (dialog->selection_vbox, FALSE);

		gedit_debug_message (DEBUG_PLUGINS, "Selection empty");
	}

	if (chars == 0)
		lines = 0;

	tmp_str = g_strdup_printf("%d", lines);
	gtk_label_set_text (GTK_LABEL (dialog->selected_lines_label), tmp_str);
	g_free (tmp_str);

	tmp_str = g_strdup_printf("%d", words);
	gtk_label_set_text (GTK_LABEL (dialog->selected_words_label), tmp_str);
	g_free (tmp_str);

	tmp_str = g_strdup_printf("%d", chars);
	gtk_label_set_text (GTK_LABEL (dialog->selected_chars_label), tmp_str);
	g_free (tmp_str);

	tmp_str = g_strdup_printf("%d", chars - white_chars);
	gtk_label_set_text (GTK_LABEL (dialog->selected_chars_ns_label), tmp_str);
	g_free (tmp_str);

	tmp_str = g_strdup_printf("%d", bytes);
	gtk_label_set_text (GTK_LABEL (dialog->selected_bytes_label), tmp_str);
	g_free (tmp_str);
}

static void
docinfo_cb (GtkAction	*action,
	    GeditWindow *window)
{
	GeditDocument *doc;
	WindowData *data;

	gedit_debug (DEBUG_PLUGINS);

	data = (WindowData *) g_object_get_data (G_OBJECT (window),
						 WINDOW_DATA_KEY);

	doc = gedit_window_get_active_document (window);
	g_return_if_fail (doc != NULL);

	if (data->dialog != NULL)
	{
		gtk_window_present (GTK_WINDOW (data->dialog->dialog));
		gtk_widget_grab_focus (GTK_WIDGET (data->dialog->dialog));
	}
	else
	{
		DocInfoDialog *dialog;

		dialog = get_docinfo_dialog (window, data);
		g_return_if_fail (dialog != NULL);
		
		data->dialog = dialog;

		gtk_widget_show (GTK_WIDGET (dialog->dialog));
	}
	
	docinfo_real (doc, 
		      data->dialog);	
	selectioninfo_real (doc, 
			    data->dialog);
}

static void
docinfo_dialog_response_cb (GtkDialog	*widget,
			    gint	res_id,
			    GeditWindow *window)
{
	WindowData *data;

	gedit_debug (DEBUG_PLUGINS);
	
	data = (WindowData *) g_object_get_data (G_OBJECT (window),
						 WINDOW_DATA_KEY);

	switch (res_id)
	{
		case GTK_RESPONSE_CLOSE:
		{
			gedit_debug_message (DEBUG_PLUGINS, "GTK_RESPONSE_CLOSE");
			gtk_widget_destroy (data->dialog->dialog);

			break;
		}

		case GTK_RESPONSE_OK:
		{
			GeditDocument *doc;
			
			gedit_debug_message (DEBUG_PLUGINS, "GTK_RESPONSE_OK");
			
			doc = gedit_window_get_active_document (window);
			g_return_if_fail (doc != NULL);
			
			docinfo_real (doc,
				      data->dialog);

			selectioninfo_real (doc,
					    data->dialog);
			
			break;
		}
	}
}

static const GtkActionEntry action_entries[] =
{
	{ "DocumentStatistics",
	  NULL,
	  N_("_Document Statistics"),
	  NULL,
	  N_("Get statistical information on the current document"),
	  G_CALLBACK (docinfo_cb) }
};

static void
free_window_data (WindowData *data)
{
	g_return_if_fail (data != NULL);
	
	gedit_debug (DEBUG_PLUGINS);

	g_object_unref (data->plugin);

	g_object_unref (data->ui_action_group);
	
	if (data->dialog != NULL)
	{
		gtk_widget_destroy (data->dialog->dialog);
	}
	
	g_free (data);
}

static void
update_ui_real (GeditWindow  *window,
		WindowData   *data)
{
	GeditView *view;

	gedit_debug (DEBUG_PLUGINS);

	view = gedit_window_get_active_view (window);

	gtk_action_group_set_sensitive (data->ui_action_group,
					(view != NULL));
					
	if (data->dialog != NULL)
	{
		gtk_dialog_set_response_sensitive (GTK_DIALOG (data->dialog->dialog),
						   GTK_RESPONSE_OK,
						   (view != NULL));
	}
}

static void
gedit_docinfo_plugin_init (GeditDocInfoPlugin *plugin)
{
	gedit_debug_message (DEBUG_PLUGINS, "GeditDocInfoPlugin initializing");
}

static void
gedit_docinfo_plugin_finalize (GObject *object)
{
	gedit_debug_message (DEBUG_PLUGINS, "GeditDocInfoPlugin finalizing");

	G_OBJECT_CLASS (gedit_docinfo_plugin_parent_class)->finalize (object);
}

static void
impl_activate (GeditPlugin *plugin,
	       GeditWindow *window)
{
	GtkUIManager *manager;
	WindowData *data;
	
	gedit_debug (DEBUG_PLUGINS);

	data = g_new (WindowData, 1);

	data->plugin = g_object_ref (plugin);
	data->dialog = NULL;
	data->ui_action_group = gtk_action_group_new ("GeditDocInfoPluginActions");
	
	gtk_action_group_set_translation_domain (data->ui_action_group, 
						 GETTEXT_PACKAGE);
	gtk_action_group_add_actions (data->ui_action_group,
				      action_entries,
				      G_N_ELEMENTS (action_entries),
				      window);

	manager = gedit_window_get_ui_manager (window);
	gtk_ui_manager_insert_action_group (manager,
					    data->ui_action_group,
					    -1);

	data->ui_id = gtk_ui_manager_new_merge_id (manager);

	g_object_set_data_full (G_OBJECT (window), 
				WINDOW_DATA_KEY, 
				data,
				(GDestroyNotify) free_window_data);

	gtk_ui_manager_add_ui (manager, 
			       data->ui_id, 
			       MENU_PATH,
			       "DocumentStatistics", 
			       "DocumentStatistics",
			       GTK_UI_MANAGER_MENUITEM, 
			       FALSE);

	update_ui_real (window,
			data);
}

static void
impl_deactivate	(GeditPlugin *plugin,
		 GeditWindow *window)
{
	GtkUIManager *manager;
	WindowData *data;

	gedit_debug (DEBUG_PLUGINS);

	manager = gedit_window_get_ui_manager (window);

	data = (WindowData *) g_object_get_data (G_OBJECT (window),
						 WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	gtk_ui_manager_remove_ui (manager,
				  data->ui_id);
	gtk_ui_manager_remove_action_group (manager,
					    data->ui_action_group);

	g_object_set_data (G_OBJECT (window),
			   WINDOW_DATA_KEY,
			   NULL);
}

static void
impl_update_ui (GeditPlugin *plugin,
		GeditWindow *window)
{
	WindowData *data;

	gedit_debug (DEBUG_PLUGINS);

	data = (WindowData *) g_object_get_data (G_OBJECT (window),
						 WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	update_ui_real (window,
			data);
}

static void
gedit_docinfo_plugin_class_init (GeditDocInfoPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GeditPluginClass *plugin_class = GEDIT_PLUGIN_CLASS (klass);

	object_class->finalize = gedit_docinfo_plugin_finalize;

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
	plugin_class->update_ui = impl_update_ui;
}
