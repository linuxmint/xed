/*
 * pluma-commands-file.c
 * This file is part of pluma
 *
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi
 * Copyright (C) 2002-2005 Paolo Maggi
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

/*
 * Modified by the pluma Team, 1998-2005. See the AUTHORS file for a
 * list of people on the pluma Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h> /* For strlen and strcmp */

#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include "pluma-commands.h"
#include "pluma-window.h"
#include "pluma-window-private.h"
#include "pluma-statusbar.h"
#include "pluma-debug.h"
#include "pluma-utils.h"
#include "pluma-file-chooser-dialog.h"
#include "dialogs/pluma-close-confirmation-dialog.h"


/* Defined constants */
#define PLUMA_OPEN_DIALOG_KEY 		"pluma-open-dialog-key"
#define PLUMA_TAB_TO_SAVE_AS  		"pluma-tab-to-save-as"
#define PLUMA_LIST_OF_TABS_TO_SAVE_AS   "pluma-list-of-tabs-to-save-as"
#define PLUMA_IS_CLOSING_ALL            "pluma-is-closing-all"
#define PLUMA_IS_QUITTING 	        "pluma-is-quitting"
#define PLUMA_IS_CLOSING_TAB		"pluma-is-closing-tab"
#define PLUMA_IS_QUITTING_ALL		"pluma-is-quitting-all"

static void tab_state_changed_while_saving (PlumaTab    *tab,
					    GParamSpec  *pspec,
					    PlumaWindow *window);

void
_pluma_cmd_file_new (GtkAction   *action,
		     PlumaWindow *window)
{
	pluma_debug (DEBUG_COMMANDS);

	pluma_window_create_tab (window, TRUE);
}

static PlumaTab *
get_tab_from_file (GList *docs, GFile *file)
{
	PlumaTab *tab = NULL;

	while (docs != NULL)
	{
		PlumaDocument *d;
		GFile *l;

		d = PLUMA_DOCUMENT (docs->data);

		l = pluma_document_get_location (d);
		if (l != NULL)
		{
			if (g_file_equal (l, file))
			{
				tab = pluma_tab_get_from_document (d);
				g_object_unref (l);
				break;
			}

			g_object_unref (l);
		}

		docs = g_list_next (docs);
	}

	return tab;
}

static gboolean
is_duplicated_file (GSList *files, GFile *file)
{
	while (files != NULL)
	{
		if (g_file_equal (files->data, file))
			return TRUE;

		files = g_slist_next (files);
	}

	return FALSE;
}

/* File loading */
static gint
load_file_list (PlumaWindow         *window,
		GSList              *files,
		const PlumaEncoding *encoding,
		gint                 line_pos,
		gboolean             create)
{
	PlumaTab      *tab;
	gint           loaded_files = 0; /* Number of files to load */
	gboolean       jump_to = TRUE; /* Whether to jump to the new tab */
	GList         *win_docs;
	GSList        *files_to_load = NULL;
	GSList        *l;

	pluma_debug (DEBUG_COMMANDS);

	win_docs = pluma_window_get_documents (window);

	/* Remove the uris corresponding to documents already open
	 * in "window" and remove duplicates from "uris" list */
	for (l = files; l != NULL; l = l->next)
	{
		if (!is_duplicated_file (files_to_load, l->data))
		{
			tab = get_tab_from_file (win_docs, l->data);
			if (tab != NULL)
			{
				if (l == files)
				{
					pluma_window_set_active_tab (window, tab);
					jump_to = FALSE;

					if (line_pos > 0)
					{
						PlumaDocument *doc;
						PlumaView *view;

						doc = pluma_tab_get_document (tab);
						view = pluma_tab_get_view (tab);

						/* document counts lines starting from 0 */
						pluma_document_goto_line (doc, line_pos - 1);
						pluma_view_scroll_to_cursor (view);
					}
				}

				++loaded_files;
			}
			else
			{
				files_to_load = g_slist_prepend (files_to_load, 
								 l->data);
			}
		}
	}

	g_list_free (win_docs);

	if (files_to_load == NULL)
		return loaded_files;
	
	files_to_load = g_slist_reverse (files_to_load);
	l = files_to_load;

	tab = pluma_window_get_active_tab (window);
	if (tab != NULL)
	{
		PlumaDocument *doc;

		doc = pluma_tab_get_document (tab);

		if (pluma_document_is_untouched (doc) &&
		    (pluma_tab_get_state (tab) == PLUMA_TAB_STATE_NORMAL))
		{
			gchar *uri;

			// FIXME: pass the GFile to tab when api is there
			uri = g_file_get_uri (l->data);
			_pluma_tab_load (tab,
					 uri,
					 encoding,
					 line_pos,
					 create);
			g_free (uri);

			l = g_slist_next (l);
			jump_to = FALSE;

			++loaded_files;
		}
	}

	while (l != NULL)
	{
		gchar *uri;

		g_return_val_if_fail (l->data != NULL, 0);

		// FIXME: pass the GFile to tab when api is there
		uri = g_file_get_uri (l->data);
		tab = pluma_window_create_tab_from_uri (window,
							uri,
							encoding,
							line_pos,
							create,
							jump_to);
		g_free (uri);

		if (tab != NULL)
		{
			jump_to = FALSE;
			++loaded_files;
		}

		l = g_slist_next (l);
	}

	if (loaded_files == 1)
	{
		PlumaDocument *doc;
		gchar *uri_for_display;

		g_return_val_if_fail (tab != NULL, loaded_files);

		doc = pluma_tab_get_document (tab);
		uri_for_display = pluma_document_get_uri_for_display (doc);

		pluma_statusbar_flash_message (PLUMA_STATUSBAR (window->priv->statusbar),
					       window->priv->generic_message_cid,
					       _("Loading file '%s'\342\200\246"),
					       uri_for_display);

		g_free (uri_for_display);
	}
	else
	{
		pluma_statusbar_flash_message (PLUMA_STATUSBAR (window->priv->statusbar),
					       window->priv->generic_message_cid,
					       ngettext("Loading %d file\342\200\246",
							"Loading %d files\342\200\246",
							loaded_files),
					       loaded_files);
	}

	/* Free uris_to_load. Note that l points to the first element of uris_to_load */
	g_slist_free (files_to_load);

	return loaded_files;
}


// FIXME: we should expose API with GFile and just make the uri
// variants backward compat wrappers

static gint
load_uri_list (PlumaWindow         *window,
	       const GSList        *uris,
	       const PlumaEncoding *encoding,
	       gint                 line_pos,
	       gboolean             create)
{
	GSList *files = NULL;
	const GSList *u;
	gint ret;

	for (u = uris; u != NULL; u = u->next)
	{
		gchar *uri = u->data;

		if (pluma_utils_is_valid_uri (uri))
			files = g_slist_prepend (files, g_file_new_for_uri (uri));
		else
			g_warning ("invalid uri: %s", uri);
	}
	files = g_slist_reverse (files);

	ret = load_file_list (window, files, encoding, line_pos, create);

	g_slist_foreach (files, (GFunc) g_object_unref, NULL);
	g_slist_free (files);

	return ret;
}

/**
 * pluma_commands_load_uri:
 *
 * Do nothing if URI does not exist
 */
void
pluma_commands_load_uri (PlumaWindow         *window,
			 const gchar         *uri,
			 const PlumaEncoding *encoding,
			 gint                 line_pos)
{
	GSList *uris = NULL;

	g_return_if_fail (PLUMA_IS_WINDOW (window));
	g_return_if_fail (uri != NULL);
	g_return_if_fail (pluma_utils_is_valid_uri (uri));

	pluma_debug_message (DEBUG_COMMANDS, "Loading URI '%s'", uri);

	uris = g_slist_prepend (uris, (gchar *)uri);

	load_uri_list (window, uris, encoding, line_pos, FALSE);

	g_slist_free (uris);
}

/**
 * pluma_commands_load_uris:
 *
 * Ignore non-existing URIs 
 */
gint
pluma_commands_load_uris (PlumaWindow         *window,
			  const GSList        *uris,
			  const PlumaEncoding *encoding,
			  gint                 line_pos)
{
	g_return_val_if_fail (PLUMA_IS_WINDOW (window), 0);
	g_return_val_if_fail ((uris != NULL) && (uris->data != NULL), 0);

	pluma_debug (DEBUG_COMMANDS);

	return load_uri_list (window, uris, encoding, line_pos, FALSE);
}

/*
 * This should become public once we convert all api to GFile:
 */
static gint
pluma_commands_load_files (PlumaWindow         *window,
			   GSList              *files,
			   const PlumaEncoding *encoding,
			   gint                 line_pos)
{
	g_return_val_if_fail (PLUMA_IS_WINDOW (window), 0);
	g_return_val_if_fail ((files != NULL) && (files->data != NULL), 0);

	pluma_debug (DEBUG_COMMANDS);

	return load_file_list (window, files, encoding, line_pos, FALSE);
}

/*
 * From the command line we can specify a line position for the
 * first doc. Beside specifying a not existing uri creates a
 * titled document.
 */
gint
_pluma_cmd_load_files_from_prompt (PlumaWindow         *window,
				   GSList              *files,
				   const PlumaEncoding *encoding,
				   gint                 line_pos)
{
	pluma_debug (DEBUG_COMMANDS);

	return load_file_list (window, files, encoding, line_pos, TRUE);
}

static void
open_dialog_destroyed (PlumaWindow            *window,
		       PlumaFileChooserDialog *dialog)
{
	pluma_debug (DEBUG_COMMANDS);

	g_object_set_data (G_OBJECT (window),
			   PLUMA_OPEN_DIALOG_KEY,
			   NULL);
}

static void
open_dialog_response_cb (PlumaFileChooserDialog *dialog,
                         gint                    response_id,
                         PlumaWindow            *window)
{
	GSList *files;
	const PlumaEncoding *encoding;

	pluma_debug (DEBUG_COMMANDS);

	if (response_id != GTK_RESPONSE_OK)
	{
		gtk_widget_destroy (GTK_WIDGET (dialog));

		return;
	}

	files = gtk_file_chooser_get_files (GTK_FILE_CHOOSER (dialog));
	g_return_if_fail (files != NULL);

	encoding = pluma_file_chooser_dialog_get_encoding (dialog);

	gtk_widget_destroy (GTK_WIDGET (dialog));

	/* Remember the folder we navigated to */
	 _pluma_window_set_default_location (window, files->data);

	pluma_commands_load_files (window,
				   files,
				   encoding,
				   0);

	g_slist_foreach (files, (GFunc) g_object_unref, NULL);
	g_slist_free (files);
}

void
_pluma_cmd_file_open (GtkAction   *action,
		      PlumaWindow *window)
{
	GtkWidget *open_dialog;
	gpointer data;
	PlumaDocument *doc;
	GFile *default_path = NULL;

	pluma_debug (DEBUG_COMMANDS);

	data = g_object_get_data (G_OBJECT (window), PLUMA_OPEN_DIALOG_KEY);

	if (data != NULL)
	{
		g_return_if_fail (PLUMA_IS_FILE_CHOOSER_DIALOG (data));

		gtk_window_present (GTK_WINDOW (data));

		return;
	}

	/* Translators: "Open Files" is the title of the file chooser window */ 
	open_dialog = pluma_file_chooser_dialog_new (_("Open Files"),
						     GTK_WINDOW (window),
						     GTK_FILE_CHOOSER_ACTION_OPEN,
						     NULL,
						     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						     GTK_STOCK_OPEN, GTK_RESPONSE_OK,
						     NULL);

	g_object_set_data (G_OBJECT (window),
			   PLUMA_OPEN_DIALOG_KEY,
			   open_dialog);

	g_object_weak_ref (G_OBJECT (open_dialog),
			   (GWeakNotify) open_dialog_destroyed,
			   window);

	/* Set the curret folder uri */
	doc = pluma_window_get_active_document (window);
	if (doc != NULL)
	{
		GFile *file;

		file = pluma_document_get_location (doc);

		if (file != NULL)
		{
			default_path = g_file_get_parent (file);
			g_object_unref (file);
		}
	}

	if (default_path == NULL)
		default_path = _pluma_window_get_default_location (window);

	if (default_path != NULL)
	{
		gchar *uri;

		uri = g_file_get_uri (default_path);
		gtk_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER (open_dialog),
							 uri);

		g_free (uri);
		g_object_unref (default_path);
	}

	g_signal_connect (open_dialog,
			  "response",
			  G_CALLBACK (open_dialog_response_cb),
			  window);

	gtk_widget_show (open_dialog);
}

/* File saving */
static void file_save_as (PlumaTab *tab, PlumaWindow *window);

static gboolean
is_read_only (GFile *location)
{
	gboolean ret = TRUE; /* default to read only */
	GFileInfo *info;

	pluma_debug (DEBUG_COMMANDS);

	info = g_file_query_info (location,
				  G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE,
				  G_FILE_QUERY_INFO_NONE,
				  NULL,
				  NULL);

	if (info != NULL)
	{
		if (g_file_info_has_attribute (info,
					       G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE))
		{
			ret = !g_file_info_get_attribute_boolean (info,
								  G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);
		}

		g_object_unref (info);
	}

	return ret;
}

/* FIXME: modify this dialog to be similar to the one provided by gtk+ for
 * already existing files - Paolo (Oct. 11, 2005) */
static gboolean
replace_read_only_file (GtkWindow *parent, GFile *file)
{
	GtkWidget *dialog;
	gint ret;
	gchar *parse_name;
	gchar *name_for_display;

	pluma_debug (DEBUG_COMMANDS);

	parse_name = g_file_get_parse_name (file);

	/* Truncate the name so it doesn't get insanely wide. Note that even
	 * though the dialog uses wrapped text, if the name doesn't contain
	 * white space then the text-wrapping code is too stupid to wrap it.
	 */
	name_for_display = pluma_utils_str_middle_truncate (parse_name, 50);
	g_free (parse_name);

	dialog = gtk_message_dialog_new (parent,
					 GTK_DIALOG_DESTROY_WITH_PARENT,
					 GTK_MESSAGE_QUESTION,
					 GTK_BUTTONS_NONE,
					 _("The file \"%s\" is read-only."),
				         name_for_display);
	g_free (name_for_display);

	gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
						  _("Do you want to try to replace it "
						    "with the one you are saving?"));

	gtk_dialog_add_button (GTK_DIALOG (dialog),
			       GTK_STOCK_CANCEL,
			       GTK_RESPONSE_CANCEL);

	pluma_dialog_add_button (GTK_DIALOG (dialog),
				 _("_Replace"),
			  	 GTK_STOCK_SAVE_AS,
			  	 GTK_RESPONSE_YES);

	gtk_dialog_set_default_response	(GTK_DIALOG (dialog),
					 GTK_RESPONSE_CANCEL);

	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

	ret = gtk_dialog_run (GTK_DIALOG (dialog));

	gtk_widget_destroy (dialog);

	return (ret == GTK_RESPONSE_YES);
}

static void
save_dialog_response_cb (PlumaFileChooserDialog *dialog,
                         gint                    response_id,
                         PlumaWindow            *window)
{
	GFile *file;
	const PlumaEncoding *encoding;
	PlumaTab *tab;
	gpointer data;
	GSList *tabs_to_save_as;
	PlumaDocumentNewlineType newline_type;

	pluma_debug (DEBUG_COMMANDS);

	tab = PLUMA_TAB (g_object_get_data (G_OBJECT (dialog),
					    PLUMA_TAB_TO_SAVE_AS));

	if (response_id != GTK_RESPONSE_OK)
	{
		gtk_widget_destroy (GTK_WIDGET (dialog));

		goto save_next_tab;
	}

	file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (dialog));
	g_return_if_fail (file != NULL);

	encoding = pluma_file_chooser_dialog_get_encoding (dialog);
	newline_type = pluma_file_chooser_dialog_get_newline_type (dialog);

	gtk_widget_destroy (GTK_WIDGET (dialog));

	if (tab != NULL)
	{
		PlumaDocument *doc;
		gchar *parse_name;
		gchar *uri;

		doc = pluma_tab_get_document (tab);
		g_return_if_fail (PLUMA_IS_DOCUMENT (doc));

		parse_name = g_file_get_parse_name (file);

		pluma_statusbar_flash_message (PLUMA_STATUSBAR (window->priv->statusbar),
					        window->priv->generic_message_cid,
					       _("Saving file '%s'\342\200\246"),
					       parse_name);

		g_free (parse_name);

		/* let's remember the dir we navigated too,
		 * even if the saving fails... */
		 _pluma_window_set_default_location (window, file);

		// FIXME: pass the GFile to tab when api is there
		uri = g_file_get_uri (file);
		_pluma_tab_save_as (tab, uri, encoding, newline_type);
		g_free (uri);
	}

	g_object_unref (file);

save_next_tab:

	data = g_object_get_data (G_OBJECT (window),
				  PLUMA_LIST_OF_TABS_TO_SAVE_AS);
	if (data == NULL)
		return;

	/* Save As the next tab of the list (we are Saving All files) */
	tabs_to_save_as = (GSList *)data;
	g_return_if_fail (tab == PLUMA_TAB (tabs_to_save_as->data));

	/* Remove the first item of the list */
	tabs_to_save_as = g_slist_delete_link (tabs_to_save_as,
					       tabs_to_save_as);

	g_object_set_data (G_OBJECT (window),
			   PLUMA_LIST_OF_TABS_TO_SAVE_AS,
			   tabs_to_save_as);

	if (tabs_to_save_as != NULL)
	{
		tab = PLUMA_TAB (tabs_to_save_as->data);

		if (GPOINTER_TO_BOOLEAN (g_object_get_data (G_OBJECT (tab),
							    PLUMA_IS_CLOSING_TAB)) == TRUE)
		{
			g_object_set_data (G_OBJECT (tab),
					   PLUMA_IS_CLOSING_TAB,
					   NULL);

			/* Trace tab state changes */
			g_signal_connect (tab,
					  "notify::state",
					  G_CALLBACK (tab_state_changed_while_saving),
					  window);
		}

		pluma_window_set_active_tab (window, tab);
		file_save_as (tab, window);
	}
}

static GtkFileChooserConfirmation
confirm_overwrite_callback (GtkFileChooser *dialog,
			    gpointer        data)
{
	gchar *uri;
	GFile *file;
	GtkFileChooserConfirmation res;

	pluma_debug (DEBUG_COMMANDS);

	uri = gtk_file_chooser_get_uri (dialog);
	file = g_file_new_for_uri (uri);
	g_free (uri);

	if (is_read_only (file))
	{
		if (replace_read_only_file (GTK_WINDOW (dialog), file))
			res = GTK_FILE_CHOOSER_CONFIRMATION_ACCEPT_FILENAME;
		else
			res = GTK_FILE_CHOOSER_CONFIRMATION_SELECT_AGAIN;
	}
	else
	{
		/* fall back to the default confirmation dialog */
		res = GTK_FILE_CHOOSER_CONFIRMATION_CONFIRM;
	}

	g_object_unref (file);

	return res;
}

static void
file_save_as (PlumaTab    *tab,
	      PlumaWindow *window)
{
	GtkWidget *save_dialog;
	GtkWindowGroup *wg;
	PlumaDocument *doc;
	GFile *file;
	gboolean uri_set = FALSE;
	const PlumaEncoding *encoding;
	PlumaDocumentNewlineType newline_type;

	g_return_if_fail (PLUMA_IS_TAB (tab));
	g_return_if_fail (PLUMA_IS_WINDOW (window));

	pluma_debug (DEBUG_COMMANDS);

	save_dialog = pluma_file_chooser_dialog_new (_("Save As\342\200\246"),
						     GTK_WINDOW (window),
						     GTK_FILE_CHOOSER_ACTION_SAVE,
						     NULL,
						     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						     GTK_STOCK_SAVE, GTK_RESPONSE_OK,
						     NULL);

	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (save_dialog),
							TRUE);
	g_signal_connect (save_dialog,
			  "confirm-overwrite",
			  G_CALLBACK (confirm_overwrite_callback),
			  NULL);

	wg = pluma_window_get_group (window);

	gtk_window_group_add_window (wg,
				     GTK_WINDOW (save_dialog));

	/* Save As dialog is modal to its main window */
	gtk_window_set_modal (GTK_WINDOW (save_dialog), TRUE);

	/* Set the suggested file name */
	doc = pluma_tab_get_document (tab);
	file = pluma_document_get_location (doc);

	if (file != NULL)
	{
		uri_set = gtk_file_chooser_set_file (GTK_FILE_CHOOSER (save_dialog),
						     file,
						     NULL);

		g_object_unref (file);
	}


	if (!uri_set)
	{
		GFile *default_path;
		gchar *docname;

		default_path = _pluma_window_get_default_location (window);
		docname = pluma_document_get_short_name_for_display (doc);

		if (default_path != NULL)
		{
			gchar *uri;

			uri = g_file_get_uri (default_path);
			gtk_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER (save_dialog),
								 uri);

			g_free (uri);
			g_object_unref (default_path);
		}

		gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (save_dialog),
						   docname);

		g_free (docname);
	}

	/* Set suggested encoding */
	encoding = pluma_document_get_encoding (doc);
	g_return_if_fail (encoding != NULL);

	newline_type = pluma_document_get_newline_type (doc);

	pluma_file_chooser_dialog_set_encoding (PLUMA_FILE_CHOOSER_DIALOG (save_dialog),
						encoding);

	pluma_file_chooser_dialog_set_newline_type (PLUMA_FILE_CHOOSER_DIALOG (save_dialog),
	                                            newline_type);

	g_object_set_data (G_OBJECT (save_dialog),
			   PLUMA_TAB_TO_SAVE_AS,
			   tab);

	g_signal_connect (save_dialog,
			  "response",
			  G_CALLBACK (save_dialog_response_cb),
			  window);

	gtk_widget_show (save_dialog);
}

static void
file_save (PlumaTab    *tab,
	   PlumaWindow *window)
{
	PlumaDocument *doc;
	gchar *uri_for_display;

	pluma_debug (DEBUG_COMMANDS);

	g_return_if_fail (PLUMA_IS_TAB (tab));
	g_return_if_fail (PLUMA_IS_WINDOW (window));

	doc = pluma_tab_get_document (tab);
	g_return_if_fail (PLUMA_IS_DOCUMENT (doc));

	if (pluma_document_is_untitled (doc) || 
	    pluma_document_get_readonly (doc))
	{
		pluma_debug_message (DEBUG_COMMANDS, "Untitled or Readonly");

		file_save_as (tab, window);
		
		return;
	}

	uri_for_display = pluma_document_get_uri_for_display (doc);
	pluma_statusbar_flash_message (PLUMA_STATUSBAR (window->priv->statusbar),
				        window->priv->generic_message_cid,
				       _("Saving file '%s'\342\200\246"),
				       uri_for_display);

	g_free (uri_for_display);

	_pluma_tab_save (tab);
}

void
_pluma_cmd_file_save (GtkAction   *action,
		     PlumaWindow *window)
{
	PlumaTab *tab;

	pluma_debug (DEBUG_COMMANDS);

	tab = pluma_window_get_active_tab (window);
	if (tab == NULL)
		return;

	file_save (tab, window);
}

void
_pluma_cmd_file_save_as (GtkAction   *action,
			PlumaWindow *window)
{
	PlumaTab *tab;

	pluma_debug (DEBUG_COMMANDS);

	tab = pluma_window_get_active_tab (window);
	if (tab == NULL)
		return;

	file_save_as (tab, window);
}

static gboolean
document_needs_saving (PlumaDocument *doc)
{
	if (gtk_text_buffer_get_modified (GTK_TEXT_BUFFER (doc)))
		return TRUE;

	/* we check if it was deleted only for local files
	 * since for remote files it may hang */
	if (pluma_document_is_local (doc) && pluma_document_get_deleted (doc))
		return TRUE;

	return FALSE;
}

/*
 * The docs in the list must belong to the same PlumaWindow.
 */
void
_pluma_cmd_file_save_documents_list (PlumaWindow *window,
				     GList       *docs)
{
	GList *l;
	GSList *tabs_to_save_as = NULL;

	pluma_debug (DEBUG_COMMANDS);

	g_return_if_fail (!(pluma_window_get_state (window) & 
			    (PLUMA_WINDOW_STATE_PRINTING |
			     PLUMA_WINDOW_STATE_SAVING_SESSION)));

	l = docs;
	while (l != NULL)
	{
		PlumaDocument *doc;
		PlumaTab *t;
		PlumaTabState state;

		g_return_if_fail (PLUMA_IS_DOCUMENT (l->data));
 
		doc = PLUMA_DOCUMENT (l->data);
		t = pluma_tab_get_from_document (doc);
		state = pluma_tab_get_state (t);

		g_return_if_fail (state != PLUMA_TAB_STATE_PRINTING);
		g_return_if_fail (state != PLUMA_TAB_STATE_PRINT_PREVIEWING);
		g_return_if_fail (state != PLUMA_TAB_STATE_CLOSING);

		if ((state == PLUMA_TAB_STATE_NORMAL) ||
		    (state == PLUMA_TAB_STATE_SHOWING_PRINT_PREVIEW) ||
		    (state == PLUMA_TAB_STATE_GENERIC_NOT_EDITABLE))
		{
			/* FIXME: manage the case of local readonly files owned by the
			   user is running pluma - Paolo (Dec. 8, 2005) */
			if (pluma_document_is_untitled (doc) || 
			    pluma_document_get_readonly (doc))
			{
				if (document_needs_saving (doc))
			     	{
				     	tabs_to_save_as = g_slist_prepend (tabs_to_save_as,
									   t);
			     	}
			}
			else
			{
				file_save (t, window);			
			}
		}
		else
		{
			/* If the state is:
			   - PLUMA_TAB_STATE_LOADING: we do not save since we are sure the file is unmodified
			   - PLUMA_TAB_STATE_REVERTING: we do not save since the user wants
			     to return back to the version of the file she previously saved
			   - PLUMA_TAB_STATE_SAVING: well, we are already saving (no need to save again)
			   - PLUMA_TAB_STATE_PRINTING, PLUMA_TAB_STATE_PRINT_PREVIEWING: there is not a
			     real reason for not saving in this case, we do not save to avoid to run
			     two operations using the message area at the same time (may be we can remove
			     this limitation in the future). Note that SaveAll, ClosAll
			     and Quit are unsensitive if the window state is PRINTING.
			   - PLUMA_TAB_STATE_GENERIC_ERROR: we do not save since the document contains
			     errors (I don't think this is a very frequent case, we should probably remove
			     this state)
			   - PLUMA_TAB_STATE_LOADING_ERROR: there is nothing to save
			   - PLUMA_TAB_STATE_REVERTING_ERROR: there is nothing to save and saving the current
			     document will overwrite the copy of the file the user wants to go back to
			   - PLUMA_TAB_STATE_SAVING_ERROR: we do not save since we just failed to save, so there is
			     no reason to automatically retry... we wait for user intervention
			   - PLUMA_TAB_STATE_CLOSING: this state is invalid in this case
			*/

			gchar *uri_for_display;

			uri_for_display = pluma_document_get_uri_for_display (doc);
			pluma_debug_message (DEBUG_COMMANDS,
					     "File '%s' not saved. State: %d",
					     uri_for_display,
					     state);
			g_free (uri_for_display);
		}

		l = g_list_next (l);
	}

	if (tabs_to_save_as != NULL)
	{
		PlumaTab *tab;

		tabs_to_save_as = g_slist_reverse (tabs_to_save_as );

		g_return_if_fail (g_object_get_data (G_OBJECT (window),
						     PLUMA_LIST_OF_TABS_TO_SAVE_AS) == NULL);

		g_object_set_data (G_OBJECT (window),
				   PLUMA_LIST_OF_TABS_TO_SAVE_AS,
				   tabs_to_save_as);

		tab = PLUMA_TAB (tabs_to_save_as->data);

		pluma_window_set_active_tab (window, tab);
		file_save_as (tab, window);
	}
}

void
pluma_commands_save_all_documents (PlumaWindow *window)
{
	GList *docs;
	
	g_return_if_fail (PLUMA_IS_WINDOW (window));

	pluma_debug (DEBUG_COMMANDS);

	docs = pluma_window_get_documents (window);

	_pluma_cmd_file_save_documents_list (window, docs);

	g_list_free (docs);
}

void
_pluma_cmd_file_save_all (GtkAction   *action,
			 PlumaWindow *window)
{
	pluma_commands_save_all_documents (window);
}

void
pluma_commands_save_document (PlumaWindow   *window,
                              PlumaDocument *document)
{
	PlumaTab *tab;

	g_return_if_fail (PLUMA_IS_WINDOW (window));
	g_return_if_fail (PLUMA_IS_DOCUMENT (document));
	
	pluma_debug (DEBUG_COMMANDS);
	
	tab = pluma_tab_get_from_document (document);
	file_save (tab, window);
}

/* File revert */
static void
do_revert (PlumaWindow *window,
	   PlumaTab    *tab)
{
	PlumaDocument *doc;
	gchar *docname;

	pluma_debug (DEBUG_COMMANDS);

	doc = pluma_tab_get_document (tab);
	docname = pluma_document_get_short_name_for_display (doc);

	pluma_statusbar_flash_message (PLUMA_STATUSBAR (window->priv->statusbar),
				        window->priv->generic_message_cid,
				       _("Reverting the document '%s'\342\200\246"),
				       docname);

	g_free (docname);

	_pluma_tab_revert (tab);
}

static void
revert_dialog_response_cb (GtkDialog   *dialog,
			   gint         response_id,
			   PlumaWindow *window)
{
	PlumaTab *tab;

	pluma_debug (DEBUG_COMMANDS);

	/* FIXME: we are relying on the fact that the dialog is
	   modal so the active tab can't be changed...
	   not very nice - Paolo (Oct 11, 2005) */
	tab = pluma_window_get_active_tab (window);
	if (tab == NULL)
		return;

	gtk_widget_destroy (GTK_WIDGET (dialog));

	if (response_id == GTK_RESPONSE_OK)
	{
		do_revert (window, tab);
	}
}

static GtkWidget *
revert_dialog (PlumaWindow   *window,
	       PlumaDocument *doc)
{
	GtkWidget *dialog;
	gchar *docname;
	gchar *primary_msg;
	gchar *secondary_msg;
	glong seconds;

	pluma_debug (DEBUG_COMMANDS);

	docname = pluma_document_get_short_name_for_display (doc);
	primary_msg = g_strdup_printf (_("Revert unsaved changes to document '%s'?"),
	                               docname);
	g_free (docname);

	seconds = MAX (1, _pluma_document_get_seconds_since_last_save_or_load (doc));

	if (seconds < 55)
	{
		secondary_msg = g_strdup_printf (
					ngettext ("Changes made to the document in the last %ld second "
					    	  "will be permanently lost.",
						  "Changes made to the document in the last %ld seconds "
					    	  "will be permanently lost.",
						  seconds),
					seconds);
	}
	else if (seconds < 75) /* 55 <= seconds < 75 */
	{
		secondary_msg = g_strdup (_("Changes made to the document in the last minute "
					    "will be permanently lost."));
	}
	else if (seconds < 110) /* 75 <= seconds < 110 */
	{
		secondary_msg = g_strdup_printf (
					ngettext ("Changes made to the document in the last minute and "
						  "%ld second will be permanently lost.",
						  "Changes made to the document in the last minute and "
						  "%ld seconds will be permanently lost.",
						  seconds - 60 ),
					seconds - 60);
	}
	else if (seconds < 3600)
	{
		secondary_msg = g_strdup_printf (
					ngettext ("Changes made to the document in the last %ld minute "
					    	  "will be permanently lost.",
						  "Changes made to the document in the last %ld minutes "
					    	  "will be permanently lost.",
						  seconds / 60),
					seconds / 60);
	}
	else if (seconds < 7200)
	{
		gint minutes;
		seconds -= 3600;

		minutes = seconds / 60;
		if (minutes < 5)
		{
			secondary_msg = g_strdup (_("Changes made to the document in the last hour "
						    "will be permanently lost."));
		}
		else
		{
			secondary_msg = g_strdup_printf (
					ngettext ("Changes made to the document in the last hour and "
						  "%d minute will be permanently lost.",
						  "Changes made to the document in the last hour and "
						  "%d minutes will be permanently lost.",
						  minutes),
					minutes);
		}
	}
	else
	{
		gint hours;

		hours = seconds / 3600;

		secondary_msg = g_strdup_printf (
					ngettext ("Changes made to the document in the last %d hour "
					    	  "will be permanently lost.",
						  "Changes made to the document in the last %d hours "
					    	  "will be permanently lost.",
						  hours),
					hours);
	}

	dialog = gtk_message_dialog_new (GTK_WINDOW (window),
					 GTK_DIALOG_DESTROY_WITH_PARENT,
					 GTK_MESSAGE_QUESTION,
					 GTK_BUTTONS_NONE,
					 "%s", primary_msg);

	gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
						  "%s", secondary_msg);
	g_free (primary_msg);
	g_free (secondary_msg);

	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

	gtk_dialog_add_button (GTK_DIALOG (dialog),
			       GTK_STOCK_CANCEL,
			       GTK_RESPONSE_CANCEL);

	pluma_dialog_add_button (GTK_DIALOG (dialog),
				 _("_Revert"),
				 GTK_STOCK_REVERT_TO_SAVED,
				 GTK_RESPONSE_OK);

	gtk_dialog_set_default_response	(GTK_DIALOG (dialog),
					 GTK_RESPONSE_CANCEL);

	return dialog;
}

void
_pluma_cmd_file_revert (GtkAction   *action,
		       PlumaWindow *window)
{
	PlumaTab       *tab;
	PlumaDocument  *doc;
	GtkWidget      *dialog;
	GtkWindowGroup *wg;

	pluma_debug (DEBUG_COMMANDS);

	tab = pluma_window_get_active_tab (window);
	g_return_if_fail (tab != NULL);

	/* If we are already displaying a notification
	 * reverting will drop local modifications, do
	 * not bug the user further */
	if (pluma_tab_get_state (tab) == PLUMA_TAB_STATE_EXTERNALLY_MODIFIED_NOTIFICATION)
	{
		do_revert (window, tab);
		return;
	}

	doc = pluma_tab_get_document (tab);
	g_return_if_fail (doc != NULL);
	g_return_if_fail (!pluma_document_is_untitled (doc));

	dialog = revert_dialog (window, doc);

	wg = pluma_window_get_group (window);

	gtk_window_group_add_window (wg, GTK_WINDOW (dialog));

	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

	g_signal_connect (dialog,
			  "response",
			  G_CALLBACK (revert_dialog_response_cb),
			  window);

	gtk_widget_show (dialog);
}

/* Close tab */
static gboolean
really_close_tab (PlumaTab *tab)
{
	GtkWidget *toplevel;
	PlumaWindow *window;

	pluma_debug (DEBUG_COMMANDS);

	g_return_val_if_fail (pluma_tab_get_state (tab) == PLUMA_TAB_STATE_CLOSING,
			      FALSE);

	toplevel = gtk_widget_get_toplevel (GTK_WIDGET (tab));
	g_return_val_if_fail (PLUMA_IS_WINDOW (toplevel), FALSE);

	window = PLUMA_WINDOW (toplevel);

	pluma_window_close_tab (window, tab);

	if (pluma_window_get_active_tab (window) == NULL)
	{
		gboolean is_quitting;

		is_quitting = GPOINTER_TO_BOOLEAN (g_object_get_data (G_OBJECT (window),
								      PLUMA_IS_QUITTING));

		if (is_quitting)
			gtk_widget_destroy (GTK_WIDGET (window));
	}

	return FALSE;
}

static void
tab_state_changed_while_saving (PlumaTab    *tab,
				GParamSpec  *pspec,
				PlumaWindow *window)
{
	PlumaTabState ts;

	ts = pluma_tab_get_state (tab);

	pluma_debug_message (DEBUG_COMMANDS, "State while saving: %d\n", ts);

	/* When the state become NORMAL, it means the saving operation is
	   finished */
	if (ts == PLUMA_TAB_STATE_NORMAL)
	{
		PlumaDocument *doc;

		g_signal_handlers_disconnect_by_func (tab,
						      G_CALLBACK (tab_state_changed_while_saving),
					      	      window);

		doc = pluma_tab_get_document (tab);
		g_return_if_fail (doc != NULL);

		/* If the saving operation failed or was interrupted, then the
		   document is still "modified" -> do not close the tab */
		if (document_needs_saving (doc))
			return;

		/* Close the document only if it has been succesfully saved.
		   Tab state is set to CLOSING (it is a state without exiting
		   transitions) and the tab is closed in a idle handler */
		_pluma_tab_mark_for_closing (tab);

		g_idle_add_full (G_PRIORITY_HIGH_IDLE,
				 (GSourceFunc)really_close_tab,
				 tab,
				 NULL);
	}
}

static void
save_and_close (PlumaTab    *tab,
		PlumaWindow *window)
{
	pluma_debug (DEBUG_COMMANDS);

	/* Trace tab state changes */
	g_signal_connect (tab,
			  "notify::state",
			  G_CALLBACK (tab_state_changed_while_saving),
			  window);

	file_save (tab, window);
}

static void
save_as_and_close (PlumaTab    *tab,
		   PlumaWindow *window)
{
	pluma_debug (DEBUG_COMMANDS);

	g_object_set_data (G_OBJECT (tab),
			   PLUMA_IS_CLOSING_TAB,
			   NULL);

	/* Trace tab state changes */
	g_signal_connect (tab,
			  "notify::state",
			  G_CALLBACK (tab_state_changed_while_saving),
			  window);

	pluma_window_set_active_tab (window, tab);
	file_save_as (tab, window);
}

static void
save_and_close_all_documents (const GList  *docs,
			      PlumaWindow  *window)
{
	GList  *tabs;
	GList  *l;
	GSList *sl;
	GSList *tabs_to_save_as;
	GSList *tabs_to_save_and_close;
	GList  *tabs_to_close;

	pluma_debug (DEBUG_COMMANDS);

	g_return_if_fail (!(pluma_window_get_state (window) & PLUMA_WINDOW_STATE_PRINTING));

	tabs = gtk_container_get_children (
			GTK_CONTAINER (_pluma_window_get_notebook (window)));

	tabs_to_save_as = NULL;
	tabs_to_save_and_close = NULL;
	tabs_to_close = NULL;

	l = tabs;
	while (l != NULL)
	{
		PlumaTab *t;
		PlumaTabState state;
		PlumaDocument *doc;

		t = PLUMA_TAB (l->data);

		state = pluma_tab_get_state (t);
		doc = pluma_tab_get_document (t);

		/* If the state is: ([*] invalid states)
		   - PLUMA_TAB_STATE_NORMAL: close (and if needed save)
		   - PLUMA_TAB_STATE_LOADING: close, we are sure the file is unmodified
		   - PLUMA_TAB_STATE_REVERTING: since the user wants
		     to return back to the version of the file she previously saved, we can close
		     without saving (CHECK: are we sure this is the right behavior, suppose the case 
		     the original file has been deleted)
		   - [*] PLUMA_TAB_STATE_SAVING: invalid, ClosAll
		     and Quit are unsensitive if the window state is SAVING.
		   - [*] PLUMA_TAB_STATE_PRINTING, PLUMA_TAB_STATE_PRINT_PREVIEWING: there is not a
		     real reason for not closing in this case, we do not save to avoid to run
		     two operations using the message area at the same time (may be we can remove
		     this limitation in the future). Note that ClosAll
		     and Quit are unsensitive if the window state is PRINTING.
		   - PLUMA_TAB_STATE_SHOWING_PRINT_PREVIEW: close (and if needed save)
		   - PLUMA_TAB_STATE_LOADING_ERROR: close without saving (if the state is LOADING_ERROR then the
		     document is not modified)
		   - PLUMA_TAB_STATE_REVERTING_ERROR: we do not close since the document contains errors
		   - PLUMA_TAB_STATE_SAVING_ERROR: we do not close since the document contains errors
		   - PLUMA_TAB_STATE_GENERIC_ERROR: we do not close since the document contains
		     errors (CHECK: we should problably remove this state)
		   - [*] PLUMA_TAB_STATE_CLOSING: this state is invalid in this case
		*/

		g_return_if_fail (state != PLUMA_TAB_STATE_PRINTING);
		g_return_if_fail (state != PLUMA_TAB_STATE_PRINT_PREVIEWING);
		g_return_if_fail (state != PLUMA_TAB_STATE_CLOSING);
		g_return_if_fail (state != PLUMA_TAB_STATE_SAVING);

		if ((state != PLUMA_TAB_STATE_SAVING_ERROR) &&
		    (state != PLUMA_TAB_STATE_GENERIC_ERROR) &&
		    (state != PLUMA_TAB_STATE_REVERTING_ERROR))
		{
			if ((g_list_index ((GList *)docs, doc) >= 0) &&
			    (state != PLUMA_TAB_STATE_LOADING) &&
			    (state != PLUMA_TAB_STATE_LOADING_ERROR) &&			    
			    (state != PLUMA_TAB_STATE_REVERTING)) /* CHECK: is this the right behavior with REVERTING ?*/
			{			
				/* The document must be saved before closing */
				g_return_if_fail (document_needs_saving (doc));
				
				/* FIXME: manage the case of local readonly files owned by the
				   user is running pluma - Paolo (Dec. 8, 2005) */
				if (pluma_document_is_untitled (doc) ||
				    pluma_document_get_readonly (doc))
				{
					g_object_set_data (G_OBJECT (t),
							   PLUMA_IS_CLOSING_TAB,
							   GBOOLEAN_TO_POINTER (TRUE));

					tabs_to_save_as = g_slist_prepend (tabs_to_save_as,
									   t);
				}
				else
				{
					tabs_to_save_and_close = g_slist_prepend (tabs_to_save_and_close,
										  t);
				}
			}
			else
			{
				/* The document must be closed without saving */
				tabs_to_close = g_list_prepend (tabs_to_close,
								t);
			}
		}

		l = g_list_next (l);
	}

	g_list_free (tabs);

	/* Close all tabs to close (in a sync way) */
	pluma_window_close_tabs (window, tabs_to_close);
	g_list_free (tabs_to_close);

	/* Save and close all the files in tabs_to_save_and_close */
	sl = tabs_to_save_and_close;
	while (sl != NULL)
	{
		save_and_close (PLUMA_TAB (sl->data),
				window);
		sl = g_slist_next (sl);
	}
	g_slist_free (tabs_to_save_and_close);

	/* Save As and close all the files in tabs_to_save_as  */
	if (tabs_to_save_as != NULL)
	{
		PlumaTab *tab;

		tabs_to_save_as = g_slist_reverse (tabs_to_save_as );

		g_return_if_fail (g_object_get_data (G_OBJECT (window),
						     PLUMA_LIST_OF_TABS_TO_SAVE_AS) == NULL);

		g_object_set_data (G_OBJECT (window),
				   PLUMA_LIST_OF_TABS_TO_SAVE_AS,
				   tabs_to_save_as);

		tab = PLUMA_TAB (tabs_to_save_as->data);

		save_as_and_close (tab, window);
	}
}

static void
save_and_close_document (const GList  *docs,
			 PlumaWindow  *window)
{
	PlumaTab *tab;

	pluma_debug (DEBUG_COMMANDS);

	g_return_if_fail (docs->next == NULL);

	tab = pluma_tab_get_from_document (PLUMA_DOCUMENT (docs->data));
	g_return_if_fail (tab != NULL);

	save_and_close (tab, window);
}

static void
close_all_tabs (PlumaWindow *window)
{
	gboolean is_quitting;

	pluma_debug (DEBUG_COMMANDS);

	/* There is no document to save -> close all tabs */
	pluma_window_close_all_tabs (window);

	is_quitting = GPOINTER_TO_BOOLEAN (g_object_get_data (G_OBJECT (window),
							      PLUMA_IS_QUITTING));

	if (is_quitting)
		gtk_widget_destroy (GTK_WIDGET (window));

	return;
}

static void
close_document (PlumaWindow   *window,
		PlumaDocument *doc)
{
	PlumaTab *tab;

	pluma_debug (DEBUG_COMMANDS);

	tab = pluma_tab_get_from_document (doc);
	g_return_if_fail (tab != NULL);

	pluma_window_close_tab (window, tab);
}

static void
close_confirmation_dialog_response_handler (PlumaCloseConfirmationDialog *dlg,
					    gint                          response_id,
					    PlumaWindow                  *window)
{
	GList *selected_documents;
	gboolean is_closing_all;

	pluma_debug (DEBUG_COMMANDS);

	is_closing_all = GPOINTER_TO_BOOLEAN (g_object_get_data (G_OBJECT (window),
					    			 PLUMA_IS_CLOSING_ALL));

	gtk_widget_hide (GTK_WIDGET (dlg));

	switch (response_id)
	{
		case GTK_RESPONSE_YES: /* Save and Close */
			selected_documents = pluma_close_confirmation_dialog_get_selected_documents (dlg);
			if (selected_documents == NULL)
			{
				if (is_closing_all)
				{
					/* There is no document to save -> close all tabs */
					/* We call gtk_widget_destroy before close_all_tabs
					 * because close_all_tabs could destroy the pluma window */
					gtk_widget_destroy (GTK_WIDGET (dlg));

					close_all_tabs (window);

					return;
				}
				else
					g_return_if_reached ();
			}
			else
			{
				if (is_closing_all)
				{
					save_and_close_all_documents (selected_documents,
								      window);
				}
				else
				{
					save_and_close_document (selected_documents,
								 window);
				}
			}

			g_list_free (selected_documents);

			break;

		case GTK_RESPONSE_NO: /* Close without Saving */
			if (is_closing_all)
			{
				/* We call gtk_widget_destroy before close_all_tabs
				 * because close_all_tabs could destroy the pluma window */
				gtk_widget_destroy (GTK_WIDGET (dlg));

				close_all_tabs (window);

				return;
			}
			else
			{
				const GList *unsaved_documents;

				unsaved_documents = pluma_close_confirmation_dialog_get_unsaved_documents (dlg);
				g_return_if_fail (unsaved_documents->next == NULL);

				close_document (window,
						PLUMA_DOCUMENT (unsaved_documents->data));
			}

			break;
		default: /* Do not close */

			/* Reset is_quitting flag */
			g_object_set_data (G_OBJECT (window),
					   PLUMA_IS_QUITTING,
					   GBOOLEAN_TO_POINTER (FALSE));

#ifdef OS_OSX
			g_object_set_data (G_OBJECT (window),
			                   PLUMA_IS_QUITTING_ALL,
			                   GINT_TO_POINTER (FALSE));
#endif
			break;
	}

	gtk_widget_destroy (GTK_WIDGET (dlg));
}

/* Returns TRUE if the tab can be immediately closed */
static gboolean
tab_can_close (PlumaTab  *tab,
	       GtkWindow *window)
{
	PlumaDocument *doc;

	pluma_debug (DEBUG_COMMANDS);

	doc = pluma_tab_get_document (tab);

	if (!_pluma_tab_can_close (tab))
	{
		GtkWidget     *dlg;

		dlg = pluma_close_confirmation_dialog_new_single (
						window,
						doc,
						FALSE);

		g_signal_connect (dlg,
				  "response",
				  G_CALLBACK (close_confirmation_dialog_response_handler),
				  window);

		gtk_widget_show (dlg);

		return FALSE;
	}

	return TRUE;
}

/* CHECK: we probably need this one public for plugins...
 * maybe even a _list variant. Or maybe it's better make
 * pluma_window_close_tab always run the confirm dialog?
 * we should not allow closing a tab without resetting the
 * PLUMA_IS_CLOSING_ALL flag!
 */
void
_pluma_cmd_file_close_tab (PlumaTab    *tab,
			   PlumaWindow *window)
{
	pluma_debug (DEBUG_COMMANDS);

	g_return_if_fail (GTK_WIDGET (window) == gtk_widget_get_toplevel (GTK_WIDGET (tab)));

	g_object_set_data (G_OBJECT (window),
			   PLUMA_IS_CLOSING_ALL,
			   GBOOLEAN_TO_POINTER (FALSE));

	g_object_set_data (G_OBJECT (window),
			   PLUMA_IS_QUITTING,
			   GBOOLEAN_TO_POINTER (FALSE));

	g_object_set_data (G_OBJECT (window), 
	                   PLUMA_IS_QUITTING_ALL, 
	                   GINT_TO_POINTER (FALSE));


	if (tab_can_close (tab, GTK_WINDOW (window)))
		pluma_window_close_tab (window, tab);
}

void
_pluma_cmd_file_close (GtkAction   *action,
		      PlumaWindow *window)
{
	PlumaTab *active_tab;

	pluma_debug (DEBUG_COMMANDS);

	active_tab = pluma_window_get_active_tab (window);

	if (active_tab == NULL)
	{
#ifdef OS_OSX
		/* Close the window on OS X */
		gtk_widget_destroy (GTK_WIDGET (window));
#endif
		return;
	}

	_pluma_cmd_file_close_tab (active_tab, window);
}

/* Close all tabs */
static void
file_close_all (PlumaWindow *window,
		gboolean     is_quitting)
{
	GList     *unsaved_docs;
	GtkWidget *dlg;

	pluma_debug (DEBUG_COMMANDS);

	g_return_if_fail (!(pluma_window_get_state (window) &
	                    (PLUMA_WINDOW_STATE_SAVING |
	                     PLUMA_WINDOW_STATE_PRINTING |
	                     PLUMA_WINDOW_STATE_SAVING_SESSION)));

	g_object_set_data (G_OBJECT (window),
			   PLUMA_IS_CLOSING_ALL,
			   GBOOLEAN_TO_POINTER (TRUE));

	g_object_set_data (G_OBJECT (window),
			   PLUMA_IS_QUITTING,
			   GBOOLEAN_TO_POINTER (is_quitting));
			   
	unsaved_docs = pluma_window_get_unsaved_documents (window);

	if (unsaved_docs == NULL)
	{
		/* There is no document to save -> close all tabs */
		pluma_window_close_all_tabs (window);

		if (is_quitting)
			gtk_widget_destroy (GTK_WIDGET (window));

		return;
	}

	if (unsaved_docs->next == NULL)
	{
		/* There is only one unsaved document */
		PlumaTab      *tab;
		PlumaDocument *doc;

		doc = PLUMA_DOCUMENT (unsaved_docs->data);

		tab = pluma_tab_get_from_document (doc);
		g_return_if_fail (tab != NULL);

		pluma_window_set_active_tab (window, tab);

		dlg = pluma_close_confirmation_dialog_new_single (
						GTK_WINDOW (window),
						doc,
						FALSE);
	}
	else
	{
		dlg = pluma_close_confirmation_dialog_new (GTK_WINDOW (window),
							   unsaved_docs,
							   FALSE);
	}

	g_list_free (unsaved_docs);

	g_signal_connect (dlg,
			  "response",
			  G_CALLBACK (close_confirmation_dialog_response_handler),
			  window);

	gtk_widget_show (dlg);
}

void
_pluma_cmd_file_close_all (GtkAction   *action,
			  PlumaWindow *window)
{
	pluma_debug (DEBUG_COMMANDS);

	g_return_if_fail (!(pluma_window_get_state (window) &
	                    (PLUMA_WINDOW_STATE_SAVING |
	                    PLUMA_WINDOW_STATE_PRINTING |
	                    PLUMA_WINDOW_STATE_SAVING_SESSION)));

	file_close_all (window, FALSE);
}

/* Quit */
#ifdef OS_OSX
static void
quit_all ()
{
	GList *windows;
	GList *item;
	PlumaApp *app;

	app = pluma_app_get_default ();
	windows = g_list_copy ((GList *)pluma_app_get_windows (app));

	for (item = windows; item; item = g_list_next (item))
	{
		PlumaWindow *window = PLUMA_WINDOW (item->data);
	
		g_object_set_data (G_OBJECT (window),
		                   PLUMA_IS_QUITTING_ALL,
		                   GINT_TO_POINTER (TRUE));

		if (!(pluma_window_get_state (window) &
		                    (PLUMA_WINDOW_STATE_SAVING |
		                     PLUMA_WINDOW_STATE_PRINTING |
		                     PLUMA_WINDOW_STATE_SAVING_SESSION)))
		{
			file_close_all (window, TRUE);
		}
	}

	g_list_free (windows);
}
#endif

void
_pluma_cmd_file_quit (GtkAction   *action,
		     PlumaWindow *window)
{
	pluma_debug (DEBUG_COMMANDS);

#ifdef OS_OSX
	if (action != NULL)
	{
		quit_all ();
		return;
	}
#endif

	g_return_if_fail (!(pluma_window_get_state (window) &
	                    (PLUMA_WINDOW_STATE_SAVING |
	                     PLUMA_WINDOW_STATE_PRINTING |
	                     PLUMA_WINDOW_STATE_SAVING_SESSION)));

	file_close_all (window, TRUE);
}
