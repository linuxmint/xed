/*
 * pluma-time-plugin.c
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
 * $Id$
 */

/*
 * Modified by the pluma Team, 2002. See the AUTHORS file for a
 * list of people on the pluma Team.
 * See the ChangeLog files for a list of changes.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <time.h>

#include "pluma-time-plugin.h"
#include <pluma/pluma-help.h>

#include <glib/gi18n-lib.h>
#include <glib.h>
#include <gmodule.h>
#include <gio/gio.h>

#include <pluma/pluma-debug.h>
#include <pluma/pluma-utils.h>

#define PLUMA_TIME_PLUGIN_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), \
					      PLUMA_TYPE_TIME_PLUGIN, \
					      PlumaTimePluginPrivate))

#define WINDOW_DATA_KEY "PlumaTimePluginWindowData"
#define MENU_PATH "/MenuBar/EditMenu/EditOps_4"

/* GSettings keys */
#define TIME_SCHEMA			"org.mate.pluma.plugins.time"
#define PROMPT_TYPE_KEY		"prompt-type"
#define SELECTED_FORMAT_KEY	"selected-format"
#define CUSTOM_FORMAT_KEY	"custom-format"

#define DEFAULT_CUSTOM_FORMAT "%d/%m/%Y %H:%M:%S"

static const gchar *formats[] =
{
	"%c",
	"%x",
	"%X",
	"%x %X",
	"%Y-%m-%d %H:%M:%S",
	"%a %b %d %H:%M:%S %Z %Y",
	"%a %b %d %H:%M:%S %Y",
	"%a %d %b %Y %H:%M:%S %Z",
	"%a %d %b %Y %H:%M:%S",
	"%d/%m/%Y",
	"%d/%m/%y",
#ifndef G_OS_WIN32
	"%D", /* This one is not supported on win32 */
#endif
	"%A %d %B %Y",
	"%A %B %d %Y",
	"%Y-%m-%d",
	"%d %B %Y",
	"%B %d, %Y",
	"%A %b %d",
	"%H:%M:%S",
	"%H:%M",
	"%I:%M:%S %p",
	"%I:%M %p",
	"%H.%M.%S",
	"%H.%M",
	"%I.%M.%S %p",
	"%I.%M %p",
	"%d/%m/%Y %H:%M:%S",
	"%d/%m/%y %H:%M:%S",
#if __GLIBC__ >= 2
	"%a, %d %b %Y %H:%M:%S %z",
#endif
	NULL
};

enum
{
	COLUMN_FORMATS = 0,
	COLUMN_INDEX,
	NUM_COLUMNS
};

typedef struct _TimeConfigureDialog TimeConfigureDialog;

struct _TimeConfigureDialog
{
	GtkWidget *dialog;

	GtkWidget *list;

        /* Radio buttons to indicate what should be done */
        GtkWidget *prompt;
        GtkWidget *use_list;
        GtkWidget *custom;

	GtkWidget *custom_entry;
	GtkWidget *custom_format_example;

	/* Info needed for the response handler */
	PlumaTimePlugin *plugin;
};

typedef struct _ChooseFormatDialog ChooseFormatDialog;

struct _ChooseFormatDialog
{
	GtkWidget *dialog;

	GtkWidget *list;

        /* Radio buttons to indicate what should be done */
        GtkWidget *use_list;
        GtkWidget *custom;

        GtkWidget *custom_entry;
	GtkWidget *custom_format_example;

	/* Info needed for the response handler */
	GtkTextBuffer   *buffer;
	PlumaTimePlugin *plugin;
};

typedef enum
{
	PROMPT_SELECTED_FORMAT = 0,	/* Popup dialog with list preselected */
	PROMPT_CUSTOM_FORMAT,		/* Popup dialog with entry preselected */
	USE_SELECTED_FORMAT,		/* Use selected format directly */
	USE_CUSTOM_FORMAT		/* Use custom format directly */
} PlumaTimePluginPromptType;

struct _PlumaTimePluginPrivate
{
	GSettings *settings;
};

PLUMA_PLUGIN_REGISTER_TYPE(PlumaTimePlugin, pluma_time_plugin)

typedef struct
{
	GtkActionGroup *action_group;
	guint           ui_id;
} WindowData;

typedef struct
{
	PlumaWindow     *window;
	PlumaTimePlugin *plugin;
} ActionData;

static void time_cb (GtkAction *action, ActionData *data);

static const GtkActionEntry action_entries[] =
{
	{
		"InsertDateAndTime",
		NULL,
		N_("In_sert Date and Time..."),
		NULL,
		N_("Insert current date and time at the cursor position"),
		G_CALLBACK (time_cb)
	},
};

static void
pluma_time_plugin_init (PlumaTimePlugin *plugin)
{
	pluma_debug_message (DEBUG_PLUGINS, "PlumaTimePlugin initializing");

	plugin->priv = PLUMA_TIME_PLUGIN_GET_PRIVATE (plugin);

	plugin->priv->settings = g_settings_new (TIME_SCHEMA);
}

static void
pluma_time_plugin_finalize (GObject *object)
{
	PlumaTimePlugin *plugin = PLUMA_TIME_PLUGIN (object);

	pluma_debug_message (DEBUG_PLUGINS, "PlumaTimePlugin finalizing");

	g_object_unref (G_OBJECT (plugin->priv->settings));

	G_OBJECT_CLASS (pluma_time_plugin_parent_class)->finalize (object);
}

static void
free_window_data (WindowData *data)
{
	g_return_if_fail (data != NULL);

	g_object_unref (data->action_group);
	g_free (data);
}

static void
update_ui_real (PlumaWindow  *window,
		WindowData   *data)
{
	PlumaView *view;
	GtkAction *action;

	pluma_debug (DEBUG_PLUGINS);

	view = pluma_window_get_active_view (window);

	pluma_debug_message (DEBUG_PLUGINS, "View: %p", view);

	action = gtk_action_group_get_action (data->action_group,
					      "InsertDateAndTime");
	gtk_action_set_sensitive (action,
				  (view != NULL) &&
				  gtk_text_view_get_editable (GTK_TEXT_VIEW (view)));
}

static void
impl_activate (PlumaPlugin *plugin,
	       PlumaWindow *window)
{
	GtkUIManager *manager;
	WindowData *data;
	ActionData *action_data;

	pluma_debug (DEBUG_PLUGINS);

	data = g_new (WindowData, 1);
	action_data = g_new (ActionData, 1);

	action_data->plugin = PLUMA_TIME_PLUGIN (plugin);
	action_data->window = window;

	manager = pluma_window_get_ui_manager (window);

	data->action_group = gtk_action_group_new ("PlumaTimePluginActions");
	gtk_action_group_set_translation_domain (data->action_group,
						 GETTEXT_PACKAGE);
	gtk_action_group_add_actions_full (data->action_group,
				      	   action_entries,
				      	   G_N_ELEMENTS (action_entries),
				      	   action_data,
				      	   (GDestroyNotify) g_free);

	gtk_ui_manager_insert_action_group (manager, data->action_group, -1);

	data->ui_id = gtk_ui_manager_new_merge_id (manager);

	g_object_set_data_full (G_OBJECT (window),
				WINDOW_DATA_KEY,
				data,
				(GDestroyNotify) free_window_data);

	gtk_ui_manager_add_ui (manager,
			       data->ui_id,
			       MENU_PATH,
			       "InsertDateAndTime",
			       "InsertDateAndTime",
			       GTK_UI_MANAGER_MENUITEM,
			       FALSE);

	update_ui_real (window, data);
}

static void
impl_deactivate	(PlumaPlugin *plugin,
		 PlumaWindow *window)
{
	GtkUIManager *manager;
	WindowData *data;

	pluma_debug (DEBUG_PLUGINS);

	manager = pluma_window_get_ui_manager (window);

	data = (WindowData *) g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	gtk_ui_manager_remove_ui (manager, data->ui_id);
	gtk_ui_manager_remove_action_group (manager, data->action_group);

	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);
}

static void
impl_update_ui	(PlumaPlugin *plugin,
		 PlumaWindow *window)
{
	WindowData   *data;

	pluma_debug (DEBUG_PLUGINS);

	data = (WindowData *) g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	update_ui_real (window, data);
}

/* whether we should prompt the user or use the specified format */
static PlumaTimePluginPromptType
get_prompt_type (PlumaTimePlugin *plugin)
{
	PlumaTimePluginPromptType prompt_type;

	prompt_type = g_settings_get_enum (plugin->priv->settings,
			        	       PROMPT_TYPE_KEY);

	return prompt_type;
}

static void
set_prompt_type (PlumaTimePlugin           *plugin,
		 PlumaTimePluginPromptType  prompt_type)
{
	if (!g_settings_is_writable (plugin->priv->settings,
					   PROMPT_TYPE_KEY))
	{
		return;
	}

	g_settings_set_enum (plugin->priv->settings,
				 PROMPT_TYPE_KEY,
				 prompt_type);
}

/* The selected format in the list */
static gchar *
get_selected_format (PlumaTimePlugin *plugin)
{
	gchar *sel_format;

	sel_format = g_settings_get_string (plugin->priv->settings,
					      SELECTED_FORMAT_KEY);

	return sel_format ? sel_format : g_strdup (formats [0]);
}

static void
set_selected_format (PlumaTimePlugin *plugin,
		     const gchar     *format)
{
	g_return_if_fail (format != NULL);

	if (!g_settings_is_writable (plugin->priv->settings,
					   SELECTED_FORMAT_KEY))
	{
		return;
	}

	g_settings_set_string (plugin->priv->settings,
				 SELECTED_FORMAT_KEY,
		       		 format);
}

/* the custom format in the entry */
static gchar *
get_custom_format (PlumaTimePlugin *plugin)
{
	gchar *format;

	format = g_settings_get_string (plugin->priv->settings,
					  CUSTOM_FORMAT_KEY);

	return format ? format : g_strdup (DEFAULT_CUSTOM_FORMAT);
}

static void
set_custom_format (PlumaTimePlugin *plugin,
		   const gchar     *format)
{
	g_return_if_fail (format != NULL);

	if (!g_settings_is_writable (plugin->priv->settings,
					   CUSTOM_FORMAT_KEY))
		return;

	g_settings_set_string (plugin->priv->settings,
				 CUSTOM_FORMAT_KEY,
		       		 format);
}

static gchar *
get_time (const gchar* format)
{
  	gchar *out = NULL;
	gchar *out_utf8 = NULL;
  	time_t clock;
  	struct tm *now;
  	size_t out_length = 0;
	gchar *locale_format;

	pluma_debug (DEBUG_PLUGINS);

	g_return_val_if_fail (format != NULL, NULL);

	if (strlen (format) == 0)
		return g_strdup (" ");

	locale_format = g_locale_from_utf8 (format, -1, NULL, NULL, NULL);
	if (locale_format == NULL)
		return g_strdup (" ");

  	clock = time (NULL);
  	now = localtime (&clock);

	do
	{
		out_length += 255;
		out = g_realloc (out, out_length);
	}
  	while (strftime (out, out_length, locale_format, now) == 0);

	g_free (locale_format);

	if (g_utf8_validate (out, -1, NULL))
	{
		out_utf8 = out;
	}
	else
	{
		out_utf8 = g_locale_to_utf8 (out, -1, NULL, NULL, NULL);
		g_free (out);

		if (out_utf8 == NULL)
			out_utf8 = g_strdup (" ");
	}

  	return out_utf8;
}

static void
#if GTK_CHECK_VERSION (3, 0, 0)
dialog_disposed (GObject *obj, gpointer dialog_pointer)
#else
dialog_destroyed (GtkObject *obj, gpointer dialog_pointer)
#endif
{
	pluma_debug (DEBUG_PLUGINS);

	g_free (dialog_pointer);

	pluma_debug_message (DEBUG_PLUGINS, "END");
}

static GtkTreeModel *
create_model (GtkWidget       *listview,
	      const gchar     *sel_format,
	      PlumaTimePlugin *plugin)
{
	gint i = 0;
	GtkListStore *store;
	GtkTreeSelection *selection;
	GtkTreeIter iter;

	pluma_debug (DEBUG_PLUGINS);

	/* create list store */
	store = gtk_list_store_new (NUM_COLUMNS, G_TYPE_STRING, G_TYPE_INT);

	/* Set tree view model*/
	gtk_tree_view_set_model (GTK_TREE_VIEW (listview),
				 GTK_TREE_MODEL (store));
	g_object_unref (G_OBJECT (store));

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (listview));
	g_return_val_if_fail (selection != NULL, GTK_TREE_MODEL (store));

	/* there should always be one line selected */
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_BROWSE);

	/* add data to the list store */
	while (formats[i] != NULL)
	{
		gchar *str;

		str = get_time (formats[i]);

		pluma_debug_message (DEBUG_PLUGINS, "%d : %s", i, str);
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter,
				    COLUMN_FORMATS, str,
				    COLUMN_INDEX, i,
				    -1);
		g_free (str);

		if (sel_format && strcmp (formats[i], sel_format) == 0)
			gtk_tree_selection_select_iter (selection, &iter);

		++i;
	}

	/* fall back to select the first iter */
	if (!gtk_tree_selection_get_selected (selection, NULL, NULL))
	{
		gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), &iter);
		gtk_tree_selection_select_iter (selection, &iter);
	}

	return GTK_TREE_MODEL (store);
}

static void
scroll_to_selected (GtkTreeView *tree_view)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeIter iter;

	pluma_debug (DEBUG_PLUGINS);

	model = gtk_tree_view_get_model (tree_view);
	g_return_if_fail (model != NULL);

	/* Scroll to selected */
	selection = gtk_tree_view_get_selection (tree_view);
	g_return_if_fail (selection != NULL);

	if (gtk_tree_selection_get_selected (selection, NULL, &iter))
	{
		GtkTreePath* path;

		path = gtk_tree_model_get_path (model, &iter);
		g_return_if_fail (path != NULL);

		gtk_tree_view_scroll_to_cell (tree_view,
					      path, NULL, TRUE, 1.0, 0.0);
		gtk_tree_path_free (path);
	}
}

static void
create_formats_list (GtkWidget       *listview,
		     const gchar     *sel_format,
		     PlumaTimePlugin *plugin)
{
	GtkTreeViewColumn *column;
	GtkCellRenderer *cell;

	pluma_debug (DEBUG_PLUGINS);

	g_return_if_fail (listview != NULL);
	g_return_if_fail (sel_format != NULL);

	/* the Available formats column */
	cell = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (
			_("Available formats"),
			cell,
			"text", COLUMN_FORMATS,
			NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (listview), column);

	/* Create model, it also add model to the tree view */
	create_model (listview, sel_format, plugin);

	g_signal_connect (listview,
			  "realize",
			  G_CALLBACK (scroll_to_selected),
			  NULL);

	gtk_widget_show (listview);
}

static void
updated_custom_format_example (GtkEntry *format_entry,
			       GtkLabel *format_example)
{
	const gchar *format;
	gchar *time;
	gchar *str;
	gchar *escaped_time;

	pluma_debug (DEBUG_PLUGINS);

	g_return_if_fail (GTK_IS_ENTRY (format_entry));
	g_return_if_fail (GTK_IS_LABEL (format_example));

	format = gtk_entry_get_text (format_entry);

	time = get_time (format);
	escaped_time = g_markup_escape_text (time, -1);

	str = g_strdup_printf ("<span size=\"small\">%s</span>", escaped_time);

	gtk_label_set_markup (format_example, str);

	g_free (escaped_time);
	g_free (time);
	g_free (str);
}

static void
choose_format_dialog_button_toggled (GtkToggleButton *button,
				     ChooseFormatDialog *dialog)
{
	pluma_debug (DEBUG_PLUGINS);

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->custom)))
	{
		gtk_widget_set_sensitive (dialog->list, FALSE);
		gtk_widget_set_sensitive (dialog->custom_entry, TRUE);
		gtk_widget_set_sensitive (dialog->custom_format_example, TRUE);

		return;
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->use_list)))
	{
		gtk_widget_set_sensitive (dialog->list, TRUE);
		gtk_widget_set_sensitive (dialog->custom_entry, FALSE);
		gtk_widget_set_sensitive (dialog->custom_format_example, FALSE);

		return;
	}
}

static void
configure_dialog_button_toggled (GtkToggleButton *button, TimeConfigureDialog *dialog)
{
	pluma_debug (DEBUG_PLUGINS);

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->custom)))
	{
		gtk_widget_set_sensitive (dialog->list, FALSE);
		gtk_widget_set_sensitive (dialog->custom_entry, TRUE);
		gtk_widget_set_sensitive (dialog->custom_format_example, TRUE);

		return;
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->use_list)))
	{
		gtk_widget_set_sensitive (dialog->list, TRUE);
		gtk_widget_set_sensitive (dialog->custom_entry, FALSE);
		gtk_widget_set_sensitive (dialog->custom_format_example, FALSE);

		return;
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->prompt)))
	{
		gtk_widget_set_sensitive (dialog->list, FALSE);
		gtk_widget_set_sensitive (dialog->custom_entry, FALSE);
		gtk_widget_set_sensitive (dialog->custom_format_example, FALSE);

		return;
	}
}

static gint
get_format_from_list (GtkWidget *listview)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeIter iter;

	pluma_debug (DEBUG_PLUGINS);

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (listview));
	g_return_val_if_fail (model != NULL, 0);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (listview));
	g_return_val_if_fail (selection != NULL, 0);

	if (gtk_tree_selection_get_selected (selection, NULL, &iter))
	{
	        gint selected_value;

		gtk_tree_model_get (model, &iter, COLUMN_INDEX, &selected_value, -1);

		pluma_debug_message (DEBUG_PLUGINS, "Sel value: %d", selected_value);

	        return selected_value;
	}

	g_return_val_if_reached (0);
}

static TimeConfigureDialog *
get_configure_dialog (PlumaTimePlugin *plugin)
{
	TimeConfigureDialog *dialog = NULL;
	gchar *data_dir;
	gchar *ui_file;
	GtkWidget *content;
	GtkWidget *viewport;
	PlumaTimePluginPromptType prompt_type;
	gchar *sf, *cf;
	GtkWidget *error_widget;
	gboolean ret;
	gchar *root_objects[] = {
		"time_dialog_content",
		NULL
	};

	pluma_debug (DEBUG_PLUGINS);

	dialog = g_new0 (TimeConfigureDialog, 1);

	dialog->dialog = gtk_dialog_new_with_buttons (_("Configure insert date/time plugin..."),
						      NULL,
						      GTK_DIALOG_DESTROY_WITH_PARENT,
						      GTK_STOCK_CANCEL,
						      GTK_RESPONSE_CANCEL,
						      GTK_STOCK_OK,
						      GTK_RESPONSE_OK,
						      GTK_STOCK_HELP,
						      GTK_RESPONSE_HELP,
						      NULL);

	/* HIG defaults */
	gtk_container_set_border_width (GTK_CONTAINER (GTK_DIALOG (dialog->dialog)), 5);
	gtk_box_set_spacing (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog->dialog))),
			     2); /* 2 * 5 + 2 = 12 */
	gtk_container_set_border_width (GTK_CONTAINER (gtk_dialog_get_action_area (GTK_DIALOG (dialog->dialog))),
					5);
	gtk_box_set_spacing (GTK_BOX (gtk_dialog_get_action_area (GTK_DIALOG (dialog->dialog))), 6);

	g_return_val_if_fail (dialog->dialog != NULL, NULL);

	data_dir = pluma_plugin_get_data_dir (PLUMA_PLUGIN (plugin));
	ui_file = g_build_filename (data_dir, "pluma-time-setup-dialog.ui", NULL);
	ret = pluma_utils_get_ui_objects (ui_file,
					  root_objects,
					  &error_widget,
					  "time_dialog_content", &content,
					  "formats_viewport", &viewport,
					  "formats_tree", &dialog->list,
					  "always_prompt", &dialog->prompt,
					  "never_prompt", &dialog->use_list,
					  "use_custom", &dialog->custom,
					  "custom_entry", &dialog->custom_entry,
					  "custom_format_example", &dialog->custom_format_example,
					  NULL);

	g_free (data_dir);
	g_free (ui_file);

	if (!ret)
	{
		gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog->dialog))),
		                    error_widget,
		                    TRUE, TRUE, 0);
		gtk_container_set_border_width (GTK_CONTAINER (error_widget), 5);

		gtk_widget_show (error_widget);

		return dialog;
	}

	gtk_window_set_resizable (GTK_WINDOW (dialog->dialog), FALSE);
#if !GTK_CHECK_VERSION (3, 0, 0)
	gtk_dialog_set_has_separator (GTK_DIALOG (dialog->dialog), FALSE);
#endif

	sf = get_selected_format (plugin);
	create_formats_list (dialog->list, sf, plugin);
	g_free (sf);

	prompt_type = get_prompt_type (plugin);

	cf = get_custom_format (plugin);
     	gtk_entry_set_text (GTK_ENTRY(dialog->custom_entry), cf);
       	g_free (cf);

        if (prompt_type == USE_CUSTOM_FORMAT)
        {
	        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->custom), TRUE);

		gtk_widget_set_sensitive (dialog->list, FALSE);
		gtk_widget_set_sensitive (dialog->custom_entry, TRUE);
		gtk_widget_set_sensitive (dialog->custom_format_example, TRUE);
        }
        else if (prompt_type == USE_SELECTED_FORMAT)
        {
	        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->use_list), TRUE);

		gtk_widget_set_sensitive (dialog->list, TRUE);
		gtk_widget_set_sensitive (dialog->custom_entry, FALSE);
		gtk_widget_set_sensitive (dialog->custom_format_example, FALSE);
        }
        else
        {
	        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->prompt), TRUE);

		gtk_widget_set_sensitive (dialog->list, FALSE);
		gtk_widget_set_sensitive (dialog->custom_entry, FALSE);
		gtk_widget_set_sensitive (dialog->custom_format_example, FALSE);
        }

	updated_custom_format_example (GTK_ENTRY (dialog->custom_entry),
			GTK_LABEL (dialog->custom_format_example));

	/* setup a window of a sane size. */
	gtk_widget_set_size_request (GTK_WIDGET (viewport), 10, 200);

	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog->dialog))),
			    content, FALSE, FALSE, 0);
	g_object_unref (content);
	gtk_container_set_border_width (GTK_CONTAINER (content), 5);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog->dialog),
					 GTK_RESPONSE_OK);

	g_signal_connect (dialog->custom,
			  "toggled",
			  G_CALLBACK (configure_dialog_button_toggled),
			  dialog);
   	g_signal_connect (dialog->prompt,
			  "toggled",
			  G_CALLBACK (configure_dialog_button_toggled),
			  dialog);
	g_signal_connect (dialog->use_list,
			  "toggled",
			  G_CALLBACK (configure_dialog_button_toggled),
			  dialog);
	g_signal_connect (dialog->dialog,
#if GTK_CHECK_VERSION (3, 0, 0)
			  "dispose",
			  G_CALLBACK (dialog_disposed),
#else
			  "destroy",
			  G_CALLBACK (dialog_destroyed),
#endif
			  dialog);
	g_signal_connect (dialog->custom_entry,
			  "changed",
			  G_CALLBACK (updated_custom_format_example),
			  dialog->custom_format_example);

	return dialog;
}

static void
real_insert_time (GtkTextBuffer *buffer,
		  const gchar   *the_time)
{
	pluma_debug_message (DEBUG_PLUGINS, "Insert: %s", the_time);

	gtk_text_buffer_begin_user_action (buffer);

	gtk_text_buffer_insert_at_cursor (buffer, the_time, -1);
	gtk_text_buffer_insert_at_cursor (buffer, " ", -1);

	gtk_text_buffer_end_user_action (buffer);
}

static void
choose_format_dialog_row_activated (GtkTreeView        *list,
				    GtkTreePath        *path,
				    GtkTreeViewColumn  *column,
				    ChooseFormatDialog *dialog)
{
	gint sel_format;
	gchar *the_time;

	sel_format = get_format_from_list (dialog->list);
	the_time = get_time (formats[sel_format]);

	set_prompt_type (dialog->plugin, PROMPT_SELECTED_FORMAT);
	set_selected_format (dialog->plugin, formats[sel_format]);

	g_return_if_fail (the_time != NULL);

	real_insert_time (dialog->buffer, the_time);

	g_free (the_time);
}

static ChooseFormatDialog *
get_choose_format_dialog (GtkWindow                 *parent,
			  PlumaTimePluginPromptType  prompt_type,
			  PlumaTimePlugin           *plugin)
{
	ChooseFormatDialog *dialog;
	gchar *data_dir;
	gchar *ui_file;
	GtkWidget *error_widget;
	gboolean ret;
	gchar *sf, *cf;
	GtkWindowGroup *wg = NULL;

	if (parent != NULL)
		wg = gtk_window_get_group (parent);

	dialog = g_new0 (ChooseFormatDialog, 1);

	data_dir = pluma_plugin_get_data_dir (PLUMA_PLUGIN (plugin));
	ui_file = g_build_filename (data_dir, "pluma-time-dialog.ui", NULL);
	ret = pluma_utils_get_ui_objects (ui_file,
					  NULL,
					  &error_widget,
					  "choose_format_dialog", &dialog->dialog,
					  "choice_list", &dialog->list,
					  "use_sel_format_radiobutton", &dialog->use_list,
					  "use_custom_radiobutton", &dialog->custom,
					  "custom_entry", &dialog->custom_entry,
					  "custom_format_example", &dialog->custom_format_example,
					  NULL);

	g_free (data_dir);
	g_free (ui_file);

	if (!ret)
	{
		GtkWidget *err_dialog;

		err_dialog = gtk_dialog_new_with_buttons (NULL,
			parent,
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
			NULL);

		if (wg != NULL)
			gtk_window_group_add_window (wg, GTK_WINDOW (err_dialog));

		gtk_window_set_resizable (GTK_WINDOW (err_dialog), FALSE);
#if !GTK_CHECK_VERSION (3, 0, 0)
		gtk_dialog_set_has_separator (GTK_DIALOG (err_dialog), FALSE);
#endif
		gtk_dialog_set_default_response (GTK_DIALOG (err_dialog), GTK_RESPONSE_OK);

		gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (GTK_DIALOG (err_dialog))),
				   error_widget);

		g_signal_connect (G_OBJECT (err_dialog),
				  "response",
				  G_CALLBACK (gtk_widget_destroy),
				  NULL);

		gtk_widget_show_all (err_dialog);

		return NULL;
	}

	gtk_window_group_add_window (wg,
			     	     GTK_WINDOW (dialog->dialog));
	gtk_window_set_transient_for (GTK_WINDOW (dialog->dialog), parent);
	gtk_window_set_modal (GTK_WINDOW (dialog->dialog), TRUE);

	sf = get_selected_format (plugin);
	create_formats_list (dialog->list, sf, plugin);
	g_free (sf);

	cf = get_custom_format (plugin);
     	gtk_entry_set_text (GTK_ENTRY(dialog->custom_entry), cf);
	g_free (cf);

	updated_custom_format_example (GTK_ENTRY (dialog->custom_entry),
				       GTK_LABEL (dialog->custom_format_example));

	if (prompt_type == PROMPT_CUSTOM_FORMAT)
	{
        	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->custom), TRUE);

		gtk_widget_set_sensitive (dialog->list, FALSE);
		gtk_widget_set_sensitive (dialog->custom_entry, TRUE);
		gtk_widget_set_sensitive (dialog->custom_format_example, TRUE);
	}
	else if (prompt_type == PROMPT_SELECTED_FORMAT)
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->use_list), TRUE);

		gtk_widget_set_sensitive (dialog->list, TRUE);
		gtk_widget_set_sensitive (dialog->custom_entry, FALSE);
		gtk_widget_set_sensitive (dialog->custom_format_example, FALSE);
	}
	else
	{
		g_return_val_if_reached (NULL);
	}

	/* setup a window of a sane size. */
	gtk_widget_set_size_request (dialog->list, 10, 200);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog->dialog),
					 GTK_RESPONSE_OK);

	g_signal_connect (dialog->custom,
			  "toggled",
			  G_CALLBACK (choose_format_dialog_button_toggled),
			  dialog);
	g_signal_connect (dialog->use_list,
			  "toggled",
			  G_CALLBACK (choose_format_dialog_button_toggled),
			  dialog);
	g_signal_connect (dialog->dialog,
#if GTK_CHECK_VERSION (3, 0, 0)
			  "dispose",
			  G_CALLBACK (dialog_disposed),
#else
			  "destroy",
			  G_CALLBACK (dialog_destroyed),
#endif
			  dialog);
	g_signal_connect (dialog->custom_entry,
			  "changed",
			  G_CALLBACK (updated_custom_format_example),
			  dialog->custom_format_example);
	g_signal_connect (dialog->list,
			  "row_activated",
			  G_CALLBACK (choose_format_dialog_row_activated),
			  dialog);

	gtk_window_set_resizable (GTK_WINDOW (dialog->dialog), FALSE);

	return dialog;
}

static void
choose_format_dialog_response_cb (GtkWidget          *widget,
				  gint                response,
				  ChooseFormatDialog *dialog)
{
	switch (response)
	{
		case GTK_RESPONSE_HELP:
		{
			pluma_debug_message (DEBUG_PLUGINS, "GTK_RESPONSE_HELP");
			pluma_help_display (GTK_WINDOW (widget),
					    NULL,
					    "pluma-insert-date-time-plugin");
			break;
		}
		case GTK_RESPONSE_OK:
		{
			gchar *the_time;

			pluma_debug_message (DEBUG_PLUGINS, "GTK_RESPONSE_OK");

			/* Get the user's chosen format */
			if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->use_list)))
			{
				gint sel_format;

				sel_format = get_format_from_list (dialog->list);
				the_time = get_time (formats[sel_format]);

				set_prompt_type (dialog->plugin, PROMPT_SELECTED_FORMAT);
				set_selected_format (dialog->plugin, formats[sel_format]);
			}
			else
			{
				const gchar *format;

				format = gtk_entry_get_text (GTK_ENTRY (dialog->custom_entry));
				the_time = get_time (format);

				set_prompt_type (dialog->plugin, PROMPT_CUSTOM_FORMAT);
				set_custom_format (dialog->plugin, format);
			}

			g_return_if_fail (the_time != NULL);

			real_insert_time (dialog->buffer, the_time);
			g_free (the_time);

			gtk_widget_destroy (dialog->dialog);
			break;
		}
		case GTK_RESPONSE_CANCEL:
			pluma_debug_message (DEBUG_PLUGINS, "GTK_RESPONSE_CANCEL");
			gtk_widget_destroy (dialog->dialog);
	}
}

static void
time_cb (GtkAction  *action,
	 ActionData *data)
{
	GtkTextBuffer *buffer;
	gchar *the_time = NULL;
	PlumaTimePluginPromptType prompt_type;

	pluma_debug (DEBUG_PLUGINS);

	buffer = GTK_TEXT_BUFFER (pluma_window_get_active_document (data->window));
	g_return_if_fail (buffer != NULL);

	prompt_type = get_prompt_type (data->plugin);

        if (prompt_type == USE_CUSTOM_FORMAT)
        {
		gchar *cf = get_custom_format (data->plugin);
	        the_time = get_time (cf);
		g_free (cf);
	}
        else if (prompt_type == USE_SELECTED_FORMAT)
        {
		gchar *sf = get_selected_format (data->plugin);
	        the_time = get_time (sf);
		g_free (sf);
	}
        else
        {
		ChooseFormatDialog *dialog;

		dialog = get_choose_format_dialog (GTK_WINDOW (data->window),
						   prompt_type,
						   data->plugin);
		if (dialog != NULL)
		{
			dialog->buffer = buffer;
			dialog->plugin = data->plugin;

			g_signal_connect (dialog->dialog,
					  "response",
					  G_CALLBACK (choose_format_dialog_response_cb),
					  dialog);

			gtk_widget_show (GTK_WIDGET (dialog->dialog));
		}

		return;
	}

	g_return_if_fail (the_time != NULL);

	real_insert_time (buffer, the_time);

	g_free (the_time);
}

static void
ok_button_pressed (TimeConfigureDialog *dialog)
{
	gint sel_format;
	const gchar *custom_format;

	pluma_debug (DEBUG_PLUGINS);

	sel_format = get_format_from_list (dialog->list);

	custom_format = gtk_entry_get_text (GTK_ENTRY (dialog->custom_entry));

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->custom)))
	{
		set_prompt_type (dialog->plugin, USE_CUSTOM_FORMAT);
		set_custom_format (dialog->plugin, custom_format);
	}
	else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->use_list)))
	{
		set_prompt_type (dialog->plugin, USE_SELECTED_FORMAT);
		set_selected_format (dialog->plugin, formats [sel_format]);
	}
	else
	{
		/* Default to prompt the user with the list selected */
		set_prompt_type (dialog->plugin, PROMPT_SELECTED_FORMAT);
	}

	pluma_debug_message (DEBUG_PLUGINS, "Sel: %d", sel_format);
}

static void
configure_dialog_response_cb (GtkWidget           *widget,
			      gint                 response,
			      TimeConfigureDialog *dialog)
{
	switch (response)
	{
		case GTK_RESPONSE_HELP:
		{
			pluma_debug_message (DEBUG_PLUGINS, "GTK_RESPONSE_HELP");

			pluma_help_display (GTK_WINDOW (dialog),
					    NULL,
					    "pluma-date-time-configure");
			break;
		}
		case GTK_RESPONSE_OK:
		{
			pluma_debug_message (DEBUG_PLUGINS, "GTK_RESPONSE_OK");

			ok_button_pressed (dialog);

			gtk_widget_destroy (dialog->dialog);
			break;
		}
		case GTK_RESPONSE_CANCEL:
		{
			pluma_debug_message (DEBUG_PLUGINS, "GTK_RESPONSE_CANCEL");
			gtk_widget_destroy (dialog->dialog);
		}
	}
}

static GtkWidget *
impl_create_configure_dialog (PlumaPlugin *plugin)
{
	TimeConfigureDialog *dialog;

	dialog = get_configure_dialog (PLUMA_TIME_PLUGIN (plugin));

	dialog->plugin = PLUMA_TIME_PLUGIN (plugin);

	g_signal_connect (dialog->dialog,
			  "response",
			  G_CALLBACK (configure_dialog_response_cb),
			  dialog);

	return GTK_WIDGET (dialog->dialog);
}

static void
pluma_time_plugin_class_init (PlumaTimePluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	PlumaPluginClass *plugin_class = PLUMA_PLUGIN_CLASS (klass);

	object_class->finalize = pluma_time_plugin_finalize;

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
	plugin_class->update_ui = impl_update_ui;

	plugin_class->create_configure_dialog = impl_create_configure_dialog;

	g_type_class_add_private (object_class, sizeof (PlumaTimePluginPrivate));
}
