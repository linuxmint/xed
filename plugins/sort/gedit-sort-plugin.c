/*
 * gedit-sort-plugin.c
 * 
 * Original author: Carlo Borreo <borreo@softhome.net>
 * Ported to Gedit2 by Lee Mallabone <mate@fonicmonkey.net>
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

#include "gedit-sort-plugin.h"

#include <string.h>
#include <glib/gi18n-lib.h>
#include <gmodule.h>

#include <gedit/gedit-debug.h>
#include <gedit/gedit-utils.h>
#include <gedit/gedit-help.h>

#define GEDIT_SORT_PLUGIN_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GEDIT_TYPE_SORT_PLUGIN, GeditSortPluginPrivate))

/* Key in case the plugin ever needs any settings. */
#define SORT_BASE_KEY "/apps/gedit-2/plugins/sort"

#define WINDOW_DATA_KEY "GeditSortPluginWindowData"
#define MENU_PATH "/MenuBar/EditMenu/EditOps_6"

GEDIT_PLUGIN_REGISTER_TYPE(GeditSortPlugin, gedit_sort_plugin)

typedef struct
{
	GtkWidget *dialog;
	GtkWidget *col_num_spinbutton;
	GtkWidget *reverse_order_checkbutton;
	GtkWidget *ignore_case_checkbutton;
	GtkWidget *remove_dups_checkbutton;

	GeditDocument *doc;

	GtkTextIter start, end; /* selection */
} SortDialog;

typedef struct
{
	GtkActionGroup *ui_action_group;
	guint ui_id;
} WindowData;

typedef struct
{
	GeditPlugin *plugin;
	GeditWindow *window;
} ActionData;

typedef struct
{
	gboolean ignore_case;
	gboolean reverse_order;
	gboolean remove_duplicates;
	gint starting_column;
} SortInfo;

static void sort_cb (GtkAction *action, ActionData *action_data);
static void sort_real (SortDialog *dialog);

static const GtkActionEntry action_entries[] =
{
	{ "Sort",
	  GTK_STOCK_SORT_ASCENDING,
	  N_("S_ort..."),
	  NULL,
	  N_("Sort the current document or selection"),
	  G_CALLBACK (sort_cb) }
};

static void
sort_dialog_destroy (GtkObject *obj,
		     gpointer  dialog_pointer)
{
	gedit_debug (DEBUG_PLUGINS);

	g_slice_free (SortDialog, dialog_pointer);
}

static void
sort_dialog_response_handler (GtkDialog  *widget,
			      gint       res_id,
			      SortDialog *dialog)
{
	gedit_debug (DEBUG_PLUGINS);

	switch (res_id)
	{
		case GTK_RESPONSE_OK:
			sort_real (dialog);
			gtk_widget_destroy (dialog->dialog);
			break;

		case GTK_RESPONSE_HELP:
			gedit_help_display (GTK_WINDOW (widget),
					    NULL,
					    "gedit-sort-plugin");
			break;

		case GTK_RESPONSE_CANCEL:
			gtk_widget_destroy (dialog->dialog);
			break;
	}
}

/* NOTE: we store the current selection in the dialog since focusing
 * the text field (like the combo box) looses the documnent selection.
 * Storing the selection ONLY works because the dialog is modal */
static void
get_current_selection (GeditWindow *window, SortDialog *dialog)
{
	GeditDocument *doc;

	gedit_debug (DEBUG_PLUGINS);

	doc = gedit_window_get_active_document (window);

	if (!gtk_text_buffer_get_selection_bounds (GTK_TEXT_BUFFER (doc),
						   &dialog->start,
						   &dialog->end))
	{
		/* No selection, get the whole document. */
		gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (doc),
					    &dialog->start,
					    &dialog->end);
	}
}

static SortDialog *
get_sort_dialog (ActionData *action_data)
{
	SortDialog *dialog;
	GtkWidget *error_widget;
	gboolean ret;
	gchar *data_dir;
	gchar *ui_file;

	gedit_debug (DEBUG_PLUGINS);

	dialog = g_slice_new (SortDialog);

	data_dir = gedit_plugin_get_data_dir (action_data->plugin);
	ui_file = g_build_filename (data_dir, "sort.ui", NULL);
	g_free (data_dir);
	ret = gedit_utils_get_ui_objects (ui_file,
					  NULL,
					  &error_widget,
					  "sort_dialog", &dialog->dialog,
					  "reverse_order_checkbutton", &dialog->reverse_order_checkbutton,
					  "col_num_spinbutton", &dialog->col_num_spinbutton,
					  "ignore_case_checkbutton", &dialog->ignore_case_checkbutton,
					  "remove_dups_checkbutton", &dialog->remove_dups_checkbutton,
					  NULL);
	g_free (ui_file);

	if (!ret)
	{
		const gchar *err_message;

		err_message = gtk_label_get_label (GTK_LABEL (error_widget));
		gedit_warning (GTK_WINDOW (action_data->window),
			       "%s", err_message);

		g_free (dialog);
		gtk_widget_destroy (error_widget);

		return NULL;
	}

	gtk_dialog_set_default_response (GTK_DIALOG (dialog->dialog),
					 GTK_RESPONSE_OK);

	g_signal_connect (dialog->dialog,
			  "destroy",
			  G_CALLBACK (sort_dialog_destroy),
			  dialog);

	g_signal_connect (dialog->dialog,
			  "response",
			  G_CALLBACK (sort_dialog_response_handler),
			  dialog);

	get_current_selection (action_data->window, dialog);

	return dialog;
}

static void
sort_cb (GtkAction  *action,
	 ActionData *action_data)
{
	GeditDocument *doc;
	GtkWindowGroup *wg;
	SortDialog *dialog;

	gedit_debug (DEBUG_PLUGINS);

	doc = gedit_window_get_active_document (action_data->window);
	g_return_if_fail (doc != NULL);

	dialog = get_sort_dialog (action_data);
	g_return_if_fail (dialog != NULL);

	wg = gedit_window_get_group (action_data->window);
	gtk_window_group_add_window (wg,
				     GTK_WINDOW (dialog->dialog));

	dialog->doc = doc;

	gtk_window_set_transient_for (GTK_WINDOW (dialog->dialog),
				      GTK_WINDOW (action_data->window));
				      
	gtk_window_set_modal (GTK_WINDOW (dialog->dialog),
			      TRUE);

	gtk_widget_show (GTK_WIDGET (dialog->dialog));
}

/* Compares two strings for the sorting algorithm. Uses the UTF-8 processing
 * functions in GLib to be as correct as possible.*/
static gint
compare_algorithm (gconstpointer s1,
		   gconstpointer s2,
		   gpointer	 data)
{
	gint length1, length2;
	gint ret;
	gchar *string1, *string2;
	gchar *substring1, *substring2;
	gchar *key1, *key2;
	SortInfo *sort_info;

	gedit_debug (DEBUG_PLUGINS);

	sort_info = (SortInfo *) data;
	g_return_val_if_fail (sort_info != NULL, -1);

	if (!sort_info->ignore_case)
	{
		string1 = *((gchar **) s1);
		string2 = *((gchar **) s2);
	}
	else
	{
		string1 = g_utf8_casefold (*((gchar **) s1), -1);
		string2 = g_utf8_casefold (*((gchar **) s2), -1);
	}

	length1 = g_utf8_strlen (string1, -1);
	length2 = g_utf8_strlen (string2, -1);

	if ((length1 < sort_info->starting_column) &&
	    (length2 < sort_info->starting_column))
	{
		ret = 0;
	}
	else if (length1 < sort_info->starting_column)
	{
		ret = -1;
	}
	else if (length2 < sort_info->starting_column)
	{
		ret = 1;
	}
	else if (sort_info->starting_column < 1)
	{
		key1 = g_utf8_collate_key (string1, -1);
		key2 = g_utf8_collate_key (string2, -1);
		ret = strcmp (key1, key2);

		g_free (key1);
		g_free (key2);
	}
	else
	{
		/* A character column offset is required, so figure out
		 * the correct offset into the UTF-8 string. */
		substring1 = g_utf8_offset_to_pointer (string1, sort_info->starting_column);
		substring2 = g_utf8_offset_to_pointer (string2, sort_info->starting_column);

		key1 = g_utf8_collate_key (substring1, -1);
		key2 = g_utf8_collate_key (substring2, -1);
		ret = strcmp (key1, key2);

		g_free (key1);
		g_free (key2);
	}

	/* Do the necessary cleanup. */
	if (sort_info->ignore_case)
	{
		g_free (string1);
		g_free (string2);
	}

	if (sort_info->reverse_order)
	{
		ret = -1 * ret;
	}

	return ret;
}

static gchar *
get_line_slice (GtkTextBuffer *buf,
		gint           line)
{
	GtkTextIter start, end;
	char *ret;

	gtk_text_buffer_get_iter_at_line (buf, &start, line);
	end = start;

	if (!gtk_text_iter_ends_line (&start))
		gtk_text_iter_forward_to_line_end (&end);

	ret= gtk_text_buffer_get_slice (buf,
					  &start,
					  &end,
					  TRUE);

	g_assert (ret != NULL);

	return ret;
}

static void
sort_real (SortDialog *dialog)
{
	GeditDocument *doc;
	GtkTextIter start, end;
	gint start_line, end_line;
	gint i;
	gchar *last_row = NULL;
	gint num_lines;
	gchar **lines;
	SortInfo *sort_info;

	gedit_debug (DEBUG_PLUGINS);

	doc = dialog->doc;
	g_return_if_fail (doc != NULL);

	sort_info = g_new0 (SortInfo, 1);
	sort_info->ignore_case = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->ignore_case_checkbutton));
	sort_info->reverse_order = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->reverse_order_checkbutton));
	sort_info->remove_duplicates = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->remove_dups_checkbutton));
	sort_info->starting_column = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (dialog->col_num_spinbutton)) - 1;

	start = dialog->start;
	end = dialog->end;
	start_line = gtk_text_iter_get_line (&start);
	end_line = gtk_text_iter_get_line (&end);

	/* if we are at line start our last line is the previus one.
	 * Otherwise the last line is the current one but we try to
	 * move the iter after the line terminator */
	if (gtk_text_iter_get_line_offset (&end) == 0)
		end_line = MAX (start_line, end_line - 1);
	else
		gtk_text_iter_forward_line (&end);

	num_lines = end_line - start_line + 1;
	lines = g_new0 (gchar *, num_lines + 1);

	gedit_debug_message (DEBUG_PLUGINS, "Building list...");

	for (i = 0; i < num_lines; i++)
	{
		lines[i] = get_line_slice (GTK_TEXT_BUFFER (doc), start_line + i);
	}

	lines[num_lines] = NULL;

	gedit_debug_message (DEBUG_PLUGINS, "Sort list...");

	g_qsort_with_data (lines,
			   num_lines,
			   sizeof (gpointer),
			   compare_algorithm,
			   sort_info);

	gedit_debug_message (DEBUG_PLUGINS, "Rebuilding document...");

	gtk_source_buffer_begin_not_undoable_action (GTK_SOURCE_BUFFER (doc));

	gtk_text_buffer_delete (GTK_TEXT_BUFFER (doc),
				&start,
				&end);

	for (i = 0; i < num_lines; i++)
	{
		if (sort_info->remove_duplicates &&
		    last_row != NULL &&
		    (strcmp (last_row, lines[i]) == 0))
			continue;

		gtk_text_buffer_insert (GTK_TEXT_BUFFER (doc),
					&start,
					lines[i],
					-1);
		gtk_text_buffer_insert (GTK_TEXT_BUFFER (doc),
					&start,
					"\n",
					-1);

		last_row = lines[i];
	}

	gtk_source_buffer_end_not_undoable_action (GTK_SOURCE_BUFFER (doc));

	g_strfreev (lines);
	g_free (sort_info);

	gedit_debug_message (DEBUG_PLUGINS, "Done.");
}

static void
free_window_data (WindowData *data)
{
	g_return_if_fail (data != NULL);

	g_object_unref (data->ui_action_group);
	g_slice_free (WindowData, data);
}

static void
free_action_data (ActionData *data)
{
	g_return_if_fail (data != NULL);

	g_slice_free (ActionData, data);
}

static void
update_ui_real (GeditWindow  *window,
		WindowData   *data)
{
	GeditView *view;

	gedit_debug (DEBUG_PLUGINS);

	view = gedit_window_get_active_view (window);

	gtk_action_group_set_sensitive (data->ui_action_group,
					(view != NULL) &&
					gtk_text_view_get_editable (GTK_TEXT_VIEW (view)));
}

static void
impl_activate (GeditPlugin *plugin,
	       GeditWindow *window)
{
	GtkUIManager *manager;
	WindowData *data;
	ActionData *action_data;

	gedit_debug (DEBUG_PLUGINS);

	data = g_slice_new (WindowData);
	action_data = g_slice_new (ActionData);
	action_data->window = window;
	action_data->plugin = plugin;

	manager = gedit_window_get_ui_manager (window);

	data->ui_action_group = gtk_action_group_new ("GeditSortPluginActions");
	gtk_action_group_set_translation_domain (data->ui_action_group, 
						 GETTEXT_PACKAGE);
	gtk_action_group_add_actions_full (data->ui_action_group,
					   action_entries,
					   G_N_ELEMENTS (action_entries),
					   action_data,
					   (GDestroyNotify) free_action_data);

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
			       "Sort", 
			       "Sort",
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
gedit_sort_plugin_init (GeditSortPlugin *plugin)
{
	gedit_debug_message (DEBUG_PLUGINS, "GeditSortPlugin initializing");
}

static void
gedit_sort_plugin_finalize (GObject *object)
{
	gedit_debug_message (DEBUG_PLUGINS, "GeditSortPlugin finalizing");

	G_OBJECT_CLASS (gedit_sort_plugin_parent_class)->finalize (object);
}

static void
gedit_sort_plugin_class_init (GeditSortPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GeditPluginClass *plugin_class = GEDIT_PLUGIN_CLASS (klass);

	object_class->finalize = gedit_sort_plugin_finalize;

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
	plugin_class->update_ui = impl_update_ui;
}
