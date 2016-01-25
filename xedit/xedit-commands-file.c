/*
 * xedit-commands-file.c
 * This file is part of xedit
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
 * Modified by the xedit Team, 1998-2005. See the AUTHORS file for a
 * list of people on the xedit Team.
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

#include "xedit-commands.h"
#include "xedit-window.h"
#include "xedit-window-private.h"
#include "xedit-statusbar.h"
#include "xedit-debug.h"
#include "xedit-utils.h"
#include "xedit-file-chooser-dialog.h"
#include "dialogs/xedit-close-confirmation-dialog.h"


/* Defined constants */
#define XEDIT_OPEN_DIALOG_KEY 		"xedit-open-dialog-key"
#define XEDIT_TAB_TO_SAVE_AS  		"xedit-tab-to-save-as"
#define XEDIT_LIST_OF_TABS_TO_SAVE_AS   "xedit-list-of-tabs-to-save-as"
#define XEDIT_IS_CLOSING_ALL            "xedit-is-closing-all"
#define XEDIT_IS_QUITTING 	        "xedit-is-quitting"
#define XEDIT_IS_CLOSING_TAB		"xedit-is-closing-tab"
#define XEDIT_IS_QUITTING_ALL		"xedit-is-quitting-all"

static void tab_state_changed_while_saving (XeditTab    *tab,
					    GParamSpec  *pspec,
					    XeditWindow *window);

void
_xedit_cmd_file_new (GtkAction   *action,
		     XeditWindow *window)
{
	xedit_debug (DEBUG_COMMANDS);

	xedit_window_create_tab (window, TRUE);
}

static XeditTab *
get_tab_from_file (GList *docs, GFile *file)
{
	XeditTab *tab = NULL;

	while (docs != NULL)
	{
		XeditDocument *d;
		GFile *l;

		d = XEDIT_DOCUMENT (docs->data);

		l = xedit_document_get_location (d);
		if (l != NULL)
		{
			if (g_file_equal (l, file))
			{
				tab = xedit_tab_get_from_document (d);
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
load_file_list (XeditWindow         *window,
		GSList              *files,
		const XeditEncoding *encoding,
		gint                 line_pos,
		gboolean             create)
{
	XeditTab      *tab;
	gint           loaded_files = 0; /* Number of files to load */
	gboolean       jump_to = TRUE; /* Whether to jump to the new tab */
	GList         *win_docs;
	GSList        *files_to_load = NULL;
	GSList        *l;

	xedit_debug (DEBUG_COMMANDS);

	win_docs = xedit_window_get_documents (window);

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
					xedit_window_set_active_tab (window, tab);
					jump_to = FALSE;

					if (line_pos > 0)
					{
						XeditDocument *doc;
						XeditView *view;

						doc = xedit_tab_get_document (tab);
						view = xedit_tab_get_view (tab);

						/* document counts lines starting from 0 */
						xedit_document_goto_line (doc, line_pos - 1);
						xedit_view_scroll_to_cursor (view);
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

	tab = xedit_window_get_active_tab (window);
	if (tab != NULL)
	{
		XeditDocument *doc;

		doc = xedit_tab_get_document (tab);

		if (xedit_document_is_untouched (doc) &&
		    (xedit_tab_get_state (tab) == XEDIT_TAB_STATE_NORMAL))
		{
			gchar *uri;

			// FIXME: pass the GFile to tab when api is there
			uri = g_file_get_uri (l->data);
			_xedit_tab_load (tab,
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
		tab = xedit_window_create_tab_from_uri (window,
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
		XeditDocument *doc;
		gchar *uri_for_display;

		g_return_val_if_fail (tab != NULL, loaded_files);

		doc = xedit_tab_get_document (tab);
		uri_for_display = xedit_document_get_uri_for_display (doc);

		xedit_statusbar_flash_message (XEDIT_STATUSBAR (window->priv->statusbar),
					       window->priv->generic_message_cid,
					       _("Loading file '%s'\342\200\246"),
					       uri_for_display);

		g_free (uri_for_display);
	}
	else
	{
		xedit_statusbar_flash_message (XEDIT_STATUSBAR (window->priv->statusbar),
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
load_uri_list (XeditWindow         *window,
	       const GSList        *uris,
	       const XeditEncoding *encoding,
	       gint                 line_pos,
	       gboolean             create)
{
	GSList *files = NULL;
	const GSList *u;
	gint ret;

	for (u = uris; u != NULL; u = u->next)
	{
		gchar *uri = u->data;

		if (xedit_utils_is_valid_uri (uri))
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
 * xedit_commands_load_uri:
 * @window:
 * @uri:
 * @encoding: (allow-none):
 * @line_pos:
 *
 * Do nothing if uri does not exist
 *
 * Returns: (transfer container):
 */
void
xedit_commands_load_uri (XeditWindow         *window,
			 const gchar         *uri,
			 const XeditEncoding *encoding,
			 gint                 line_pos)
{
	GSList *uris = NULL;

	g_return_if_fail (XEDIT_IS_WINDOW (window));
	g_return_if_fail (uri != NULL);
	g_return_if_fail (xedit_utils_is_valid_uri (uri));

	xedit_debug_message (DEBUG_COMMANDS, "Loading URI '%s'", uri);

	uris = g_slist_prepend (uris, (gchar *)uri);

	load_uri_list (window, uris, encoding, line_pos, FALSE);

	g_slist_free (uris);
}

/**
 * xedit_commands_load_uris:
 * @window:
 * @uris:
 * @encoding:
 * @line_pos:
 *
 * Ignore non-existing URIs
 *
 * Returns: (transfer container):
 */
gint
xedit_commands_load_uris (XeditWindow         *window,
			  const GSList        *uris,
			  const XeditEncoding *encoding,
			  gint                 line_pos)
{
	g_return_val_if_fail (XEDIT_IS_WINDOW (window), 0);
	g_return_val_if_fail ((uris != NULL) && (uris->data != NULL), 0);

	xedit_debug (DEBUG_COMMANDS);

	return load_uri_list (window, uris, encoding, line_pos, FALSE);
}

/*
 * This should become public once we convert all api to GFile:
 */
static gint
xedit_commands_load_files (XeditWindow         *window,
			   GSList              *files,
			   const XeditEncoding *encoding,
			   gint                 line_pos)
{
	g_return_val_if_fail (XEDIT_IS_WINDOW (window), 0);
	g_return_val_if_fail ((files != NULL) && (files->data != NULL), 0);

	xedit_debug (DEBUG_COMMANDS);

	return load_file_list (window, files, encoding, line_pos, FALSE);
}

/*
 * From the command line we can specify a line position for the
 * first doc. Beside specifying a not existing uri creates a
 * titled document.
 */
gint
_xedit_cmd_load_files_from_prompt (XeditWindow         *window,
				   GSList              *files,
				   const XeditEncoding *encoding,
				   gint                 line_pos)
{
	xedit_debug (DEBUG_COMMANDS);

	return load_file_list (window, files, encoding, line_pos, TRUE);
}

static void
open_dialog_destroyed (XeditWindow            *window,
		       XeditFileChooserDialog *dialog)
{
	xedit_debug (DEBUG_COMMANDS);

	g_object_set_data (G_OBJECT (window),
			   XEDIT_OPEN_DIALOG_KEY,
			   NULL);
}

static void
open_dialog_response_cb (XeditFileChooserDialog *dialog,
                         gint                    response_id,
                         XeditWindow            *window)
{
	GSList *files;
	const XeditEncoding *encoding;

	xedit_debug (DEBUG_COMMANDS);

	if (response_id != GTK_RESPONSE_OK)
	{
		gtk_widget_destroy (GTK_WIDGET (dialog));

		return;
	}

	files = gtk_file_chooser_get_files (GTK_FILE_CHOOSER (dialog));
	g_return_if_fail (files != NULL);

	encoding = xedit_file_chooser_dialog_get_encoding (dialog);

	gtk_widget_destroy (GTK_WIDGET (dialog));

	/* Remember the folder we navigated to */
	 _xedit_window_set_default_location (window, files->data);

	xedit_commands_load_files (window,
				   files,
				   encoding,
				   0);

	g_slist_foreach (files, (GFunc) g_object_unref, NULL);
	g_slist_free (files);
}

void
_xedit_cmd_file_open (GtkAction   *action,
		      XeditWindow *window)
{
	GtkWidget *open_dialog;
	gpointer data;
	XeditDocument *doc;
	GFile *default_path = NULL;

	xedit_debug (DEBUG_COMMANDS);

	data = g_object_get_data (G_OBJECT (window), XEDIT_OPEN_DIALOG_KEY);

	if (data != NULL)
	{
		g_return_if_fail (XEDIT_IS_FILE_CHOOSER_DIALOG (data));

		gtk_window_present (GTK_WINDOW (data));

		return;
	}

	/* Translators: "Open Files" is the title of the file chooser window */ 
	open_dialog = xedit_file_chooser_dialog_new (_("Open Files"),
						     GTK_WINDOW (window),
						     GTK_FILE_CHOOSER_ACTION_OPEN,
						     NULL,
						     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						     GTK_STOCK_OPEN, GTK_RESPONSE_OK,
						     NULL);

	g_object_set_data (G_OBJECT (window),
			   XEDIT_OPEN_DIALOG_KEY,
			   open_dialog);

	g_object_weak_ref (G_OBJECT (open_dialog),
			   (GWeakNotify) open_dialog_destroyed,
			   window);

	/* Set the curret folder uri */
	doc = xedit_window_get_active_document (window);
	if (doc != NULL)
	{
		GFile *file;

		file = xedit_document_get_location (doc);

		if (file != NULL)
		{
			default_path = g_file_get_parent (file);
			g_object_unref (file);
		}
	}

	if (default_path == NULL)
		default_path = _xedit_window_get_default_location (window);

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
static void file_save_as (XeditTab *tab, XeditWindow *window);

static gboolean
is_read_only (GFile *location)
{
	gboolean ret = TRUE; /* default to read only */
	GFileInfo *info;

	xedit_debug (DEBUG_COMMANDS);

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

	xedit_debug (DEBUG_COMMANDS);

	parse_name = g_file_get_parse_name (file);

	/* Truncate the name so it doesn't get insanely wide. Note that even
	 * though the dialog uses wrapped text, if the name doesn't contain
	 * white space then the text-wrapping code is too stupid to wrap it.
	 */
	name_for_display = xedit_utils_str_middle_truncate (parse_name, 50);
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

	xedit_dialog_add_button (GTK_DIALOG (dialog),
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
save_dialog_response_cb (XeditFileChooserDialog *dialog,
                         gint                    response_id,
                         XeditWindow            *window)
{
	GFile *file;
	const XeditEncoding *encoding;
	XeditTab *tab;
	gpointer data;
	GSList *tabs_to_save_as;
	XeditDocumentNewlineType newline_type;

	xedit_debug (DEBUG_COMMANDS);

	tab = XEDIT_TAB (g_object_get_data (G_OBJECT (dialog),
					    XEDIT_TAB_TO_SAVE_AS));

	if (response_id != GTK_RESPONSE_OK)
	{
		gtk_widget_destroy (GTK_WIDGET (dialog));

		goto save_next_tab;
	}

	file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (dialog));
	g_return_if_fail (file != NULL);

	encoding = xedit_file_chooser_dialog_get_encoding (dialog);
	newline_type = xedit_file_chooser_dialog_get_newline_type (dialog);

	gtk_widget_destroy (GTK_WIDGET (dialog));

	if (tab != NULL)
	{
		XeditDocument *doc;
		gchar *parse_name;
		gchar *uri;

		doc = xedit_tab_get_document (tab);
		g_return_if_fail (XEDIT_IS_DOCUMENT (doc));

		parse_name = g_file_get_parse_name (file);

		xedit_statusbar_flash_message (XEDIT_STATUSBAR (window->priv->statusbar),
					        window->priv->generic_message_cid,
					       _("Saving file '%s'\342\200\246"),
					       parse_name);

		g_free (parse_name);

		/* let's remember the dir we navigated too,
		 * even if the saving fails... */
		 _xedit_window_set_default_location (window, file);

		// FIXME: pass the GFile to tab when api is there
		uri = g_file_get_uri (file);
		_xedit_tab_save_as (tab, uri, encoding, newline_type);
		g_free (uri);
	}

	g_object_unref (file);

save_next_tab:

	data = g_object_get_data (G_OBJECT (window),
				  XEDIT_LIST_OF_TABS_TO_SAVE_AS);
	if (data == NULL)
		return;

	/* Save As the next tab of the list (we are Saving All files) */
	tabs_to_save_as = (GSList *)data;
	g_return_if_fail (tab == XEDIT_TAB (tabs_to_save_as->data));

	/* Remove the first item of the list */
	tabs_to_save_as = g_slist_delete_link (tabs_to_save_as,
					       tabs_to_save_as);

	g_object_set_data (G_OBJECT (window),
			   XEDIT_LIST_OF_TABS_TO_SAVE_AS,
			   tabs_to_save_as);

	if (tabs_to_save_as != NULL)
	{
		tab = XEDIT_TAB (tabs_to_save_as->data);

		if (GPOINTER_TO_BOOLEAN (g_object_get_data (G_OBJECT (tab),
							    XEDIT_IS_CLOSING_TAB)) == TRUE)
		{
			g_object_set_data (G_OBJECT (tab),
					   XEDIT_IS_CLOSING_TAB,
					   NULL);

			/* Trace tab state changes */
			g_signal_connect (tab,
					  "notify::state",
					  G_CALLBACK (tab_state_changed_while_saving),
					  window);
		}

		xedit_window_set_active_tab (window, tab);
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

	xedit_debug (DEBUG_COMMANDS);

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
file_save_as (XeditTab    *tab,
	      XeditWindow *window)
{
	GtkWidget *save_dialog;
	GtkWindowGroup *wg;
	XeditDocument *doc;
	GFile *file;
	gboolean uri_set = FALSE;
	const XeditEncoding *encoding;
	XeditDocumentNewlineType newline_type;

	g_return_if_fail (XEDIT_IS_TAB (tab));
	g_return_if_fail (XEDIT_IS_WINDOW (window));

	xedit_debug (DEBUG_COMMANDS);

	save_dialog = xedit_file_chooser_dialog_new (_("Save As\342\200\246"),
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

	wg = xedit_window_get_group (window);

	gtk_window_group_add_window (wg,
				     GTK_WINDOW (save_dialog));

	/* Save As dialog is modal to its main window */
	gtk_window_set_modal (GTK_WINDOW (save_dialog), TRUE);

	/* Set the suggested file name */
	doc = xedit_tab_get_document (tab);
	file = xedit_document_get_location (doc);

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

		default_path = _xedit_window_get_default_location (window);
		docname = xedit_document_get_short_name_for_display (doc);

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
	encoding = xedit_document_get_encoding (doc);
	g_return_if_fail (encoding != NULL);

	newline_type = xedit_document_get_newline_type (doc);

	xedit_file_chooser_dialog_set_encoding (XEDIT_FILE_CHOOSER_DIALOG (save_dialog),
						encoding);

	xedit_file_chooser_dialog_set_newline_type (XEDIT_FILE_CHOOSER_DIALOG (save_dialog),
	                                            newline_type);

	g_object_set_data (G_OBJECT (save_dialog),
			   XEDIT_TAB_TO_SAVE_AS,
			   tab);

	g_signal_connect (save_dialog,
			  "response",
			  G_CALLBACK (save_dialog_response_cb),
			  window);

	gtk_widget_show (save_dialog);
}

static void
file_save (XeditTab    *tab,
	   XeditWindow *window)
{
	XeditDocument *doc;
	gchar *uri_for_display;

	xedit_debug (DEBUG_COMMANDS);

	g_return_if_fail (XEDIT_IS_TAB (tab));
	g_return_if_fail (XEDIT_IS_WINDOW (window));

	doc = xedit_tab_get_document (tab);
	g_return_if_fail (XEDIT_IS_DOCUMENT (doc));

	if (xedit_document_is_untitled (doc) || 
	    xedit_document_get_readonly (doc))
	{
		xedit_debug_message (DEBUG_COMMANDS, "Untitled or Readonly");

		file_save_as (tab, window);
		
		return;
	}

	uri_for_display = xedit_document_get_uri_for_display (doc);
	xedit_statusbar_flash_message (XEDIT_STATUSBAR (window->priv->statusbar),
				        window->priv->generic_message_cid,
				       _("Saving file '%s'\342\200\246"),
				       uri_for_display);

	g_free (uri_for_display);

	_xedit_tab_save (tab);
}

void
_xedit_cmd_file_save (GtkAction   *action,
		     XeditWindow *window)
{
	XeditTab *tab;

	xedit_debug (DEBUG_COMMANDS);

	tab = xedit_window_get_active_tab (window);
	if (tab == NULL)
		return;

	file_save (tab, window);
}

void
_xedit_cmd_file_save_as (GtkAction   *action,
			XeditWindow *window)
{
	XeditTab *tab;

	xedit_debug (DEBUG_COMMANDS);

	tab = xedit_window_get_active_tab (window);
	if (tab == NULL)
		return;

	file_save_as (tab, window);
}

static gboolean
document_needs_saving (XeditDocument *doc)
{
	if (gtk_text_buffer_get_modified (GTK_TEXT_BUFFER (doc)))
		return TRUE;

	/* we check if it was deleted only for local files
	 * since for remote files it may hang */
	if (xedit_document_is_local (doc) && xedit_document_get_deleted (doc))
		return TRUE;

	return FALSE;
}

/*
 * The docs in the list must belong to the same XeditWindow.
 */
void
_xedit_cmd_file_save_documents_list (XeditWindow *window,
				     GList       *docs)
{
	GList *l;
	GSList *tabs_to_save_as = NULL;

	xedit_debug (DEBUG_COMMANDS);

	g_return_if_fail (!(xedit_window_get_state (window) & 
			    (XEDIT_WINDOW_STATE_PRINTING |
			     XEDIT_WINDOW_STATE_SAVING_SESSION)));

	l = docs;
	while (l != NULL)
	{
		XeditDocument *doc;
		XeditTab *t;
		XeditTabState state;

		g_return_if_fail (XEDIT_IS_DOCUMENT (l->data));
 
		doc = XEDIT_DOCUMENT (l->data);
		t = xedit_tab_get_from_document (doc);
		state = xedit_tab_get_state (t);

		g_return_if_fail (state != XEDIT_TAB_STATE_PRINTING);
		g_return_if_fail (state != XEDIT_TAB_STATE_PRINT_PREVIEWING);
		g_return_if_fail (state != XEDIT_TAB_STATE_CLOSING);

		if ((state == XEDIT_TAB_STATE_NORMAL) ||
		    (state == XEDIT_TAB_STATE_SHOWING_PRINT_PREVIEW) ||
		    (state == XEDIT_TAB_STATE_GENERIC_NOT_EDITABLE))
		{
			/* FIXME: manage the case of local readonly files owned by the
			   user is running xedit - Paolo (Dec. 8, 2005) */
			if (xedit_document_is_untitled (doc) || 
			    xedit_document_get_readonly (doc))
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
			   - XEDIT_TAB_STATE_LOADING: we do not save since we are sure the file is unmodified
			   - XEDIT_TAB_STATE_REVERTING: we do not save since the user wants
			     to return back to the version of the file she previously saved
			   - XEDIT_TAB_STATE_SAVING: well, we are already saving (no need to save again)
			   - XEDIT_TAB_STATE_PRINTING, XEDIT_TAB_STATE_PRINT_PREVIEWING: there is not a
			     real reason for not saving in this case, we do not save to avoid to run
			     two operations using the message area at the same time (may be we can remove
			     this limitation in the future). Note that SaveAll, ClosAll
			     and Quit are unsensitive if the window state is PRINTING.
			   - XEDIT_TAB_STATE_GENERIC_ERROR: we do not save since the document contains
			     errors (I don't think this is a very frequent case, we should probably remove
			     this state)
			   - XEDIT_TAB_STATE_LOADING_ERROR: there is nothing to save
			   - XEDIT_TAB_STATE_REVERTING_ERROR: there is nothing to save and saving the current
			     document will overwrite the copy of the file the user wants to go back to
			   - XEDIT_TAB_STATE_SAVING_ERROR: we do not save since we just failed to save, so there is
			     no reason to automatically retry... we wait for user intervention
			   - XEDIT_TAB_STATE_CLOSING: this state is invalid in this case
			*/

			gchar *uri_for_display;

			uri_for_display = xedit_document_get_uri_for_display (doc);
			xedit_debug_message (DEBUG_COMMANDS,
					     "File '%s' not saved. State: %d",
					     uri_for_display,
					     state);
			g_free (uri_for_display);
		}

		l = g_list_next (l);
	}

	if (tabs_to_save_as != NULL)
	{
		XeditTab *tab;

		tabs_to_save_as = g_slist_reverse (tabs_to_save_as );

		g_return_if_fail (g_object_get_data (G_OBJECT (window),
						     XEDIT_LIST_OF_TABS_TO_SAVE_AS) == NULL);

		g_object_set_data (G_OBJECT (window),
				   XEDIT_LIST_OF_TABS_TO_SAVE_AS,
				   tabs_to_save_as);

		tab = XEDIT_TAB (tabs_to_save_as->data);

		xedit_window_set_active_tab (window, tab);
		file_save_as (tab, window);
	}
}

void
xedit_commands_save_all_documents (XeditWindow *window)
{
	GList *docs;
	
	g_return_if_fail (XEDIT_IS_WINDOW (window));

	xedit_debug (DEBUG_COMMANDS);

	docs = xedit_window_get_documents (window);

	_xedit_cmd_file_save_documents_list (window, docs);

	g_list_free (docs);
}

void
_xedit_cmd_file_save_all (GtkAction   *action,
			 XeditWindow *window)
{
	xedit_commands_save_all_documents (window);
}

void
xedit_commands_save_document (XeditWindow   *window,
                              XeditDocument *document)
{
	XeditTab *tab;

	g_return_if_fail (XEDIT_IS_WINDOW (window));
	g_return_if_fail (XEDIT_IS_DOCUMENT (document));
	
	xedit_debug (DEBUG_COMMANDS);
	
	tab = xedit_tab_get_from_document (document);
	file_save (tab, window);
}

/* File revert */
static void
do_revert (XeditWindow *window,
	   XeditTab    *tab)
{
	XeditDocument *doc;
	gchar *docname;

	xedit_debug (DEBUG_COMMANDS);

	doc = xedit_tab_get_document (tab);
	docname = xedit_document_get_short_name_for_display (doc);

	xedit_statusbar_flash_message (XEDIT_STATUSBAR (window->priv->statusbar),
				        window->priv->generic_message_cid,
				       _("Reverting the document '%s'\342\200\246"),
				       docname);

	g_free (docname);

	_xedit_tab_revert (tab);
}

static void
revert_dialog_response_cb (GtkDialog   *dialog,
			   gint         response_id,
			   XeditWindow *window)
{
	XeditTab *tab;

	xedit_debug (DEBUG_COMMANDS);

	/* FIXME: we are relying on the fact that the dialog is
	   modal so the active tab can't be changed...
	   not very nice - Paolo (Oct 11, 2005) */
	tab = xedit_window_get_active_tab (window);
	if (tab == NULL)
		return;

	gtk_widget_destroy (GTK_WIDGET (dialog));

	if (response_id == GTK_RESPONSE_OK)
	{
		do_revert (window, tab);
	}
}

static GtkWidget *
revert_dialog (XeditWindow   *window,
	       XeditDocument *doc)
{
	GtkWidget *dialog;
	gchar *docname;
	gchar *primary_msg;
	gchar *secondary_msg;
	glong seconds;

	xedit_debug (DEBUG_COMMANDS);

	docname = xedit_document_get_short_name_for_display (doc);
	primary_msg = g_strdup_printf (_("Revert unsaved changes to document '%s'?"),
	                               docname);
	g_free (docname);

	seconds = MAX (1, _xedit_document_get_seconds_since_last_save_or_load (doc));

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

	xedit_dialog_add_button (GTK_DIALOG (dialog),
				 _("_Revert"),
				 GTK_STOCK_REVERT_TO_SAVED,
				 GTK_RESPONSE_OK);

	gtk_dialog_set_default_response	(GTK_DIALOG (dialog),
					 GTK_RESPONSE_CANCEL);

	return dialog;
}

void
_xedit_cmd_file_revert (GtkAction   *action,
		       XeditWindow *window)
{
	XeditTab       *tab;
	XeditDocument  *doc;
	GtkWidget      *dialog;
	GtkWindowGroup *wg;

	xedit_debug (DEBUG_COMMANDS);

	tab = xedit_window_get_active_tab (window);
	g_return_if_fail (tab != NULL);

	/* If we are already displaying a notification
	 * reverting will drop local modifications, do
	 * not bug the user further */
	if (xedit_tab_get_state (tab) == XEDIT_TAB_STATE_EXTERNALLY_MODIFIED_NOTIFICATION)
	{
		do_revert (window, tab);
		return;
	}

	doc = xedit_tab_get_document (tab);
	g_return_if_fail (doc != NULL);
	g_return_if_fail (!xedit_document_is_untitled (doc));

	dialog = revert_dialog (window, doc);

	wg = xedit_window_get_group (window);

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
really_close_tab (XeditTab *tab)
{
	GtkWidget *toplevel;
	XeditWindow *window;

	xedit_debug (DEBUG_COMMANDS);

	g_return_val_if_fail (xedit_tab_get_state (tab) == XEDIT_TAB_STATE_CLOSING,
			      FALSE);

	toplevel = gtk_widget_get_toplevel (GTK_WIDGET (tab));
	g_return_val_if_fail (XEDIT_IS_WINDOW (toplevel), FALSE);

	window = XEDIT_WINDOW (toplevel);

	xedit_window_close_tab (window, tab);

	if (xedit_window_get_active_tab (window) == NULL)
	{
		gboolean is_quitting;

		is_quitting = GPOINTER_TO_BOOLEAN (g_object_get_data (G_OBJECT (window),
								      XEDIT_IS_QUITTING));

		if (is_quitting)
			gtk_widget_destroy (GTK_WIDGET (window));
	}

	return FALSE;
}

static void
tab_state_changed_while_saving (XeditTab    *tab,
				GParamSpec  *pspec,
				XeditWindow *window)
{
	XeditTabState ts;

	ts = xedit_tab_get_state (tab);

	xedit_debug_message (DEBUG_COMMANDS, "State while saving: %d\n", ts);

	/* When the state become NORMAL, it means the saving operation is
	   finished */
	if (ts == XEDIT_TAB_STATE_NORMAL)
	{
		XeditDocument *doc;

		g_signal_handlers_disconnect_by_func (tab,
						      G_CALLBACK (tab_state_changed_while_saving),
					      	      window);

		doc = xedit_tab_get_document (tab);
		g_return_if_fail (doc != NULL);

		/* If the saving operation failed or was interrupted, then the
		   document is still "modified" -> do not close the tab */
		if (document_needs_saving (doc))
			return;

		/* Close the document only if it has been succesfully saved.
		   Tab state is set to CLOSING (it is a state without exiting
		   transitions) and the tab is closed in a idle handler */
		_xedit_tab_mark_for_closing (tab);

		g_idle_add_full (G_PRIORITY_HIGH_IDLE,
				 (GSourceFunc)really_close_tab,
				 tab,
				 NULL);
	}
}

static void
save_and_close (XeditTab    *tab,
		XeditWindow *window)
{
	xedit_debug (DEBUG_COMMANDS);

	/* Trace tab state changes */
	g_signal_connect (tab,
			  "notify::state",
			  G_CALLBACK (tab_state_changed_while_saving),
			  window);

	file_save (tab, window);
}

static void
save_as_and_close (XeditTab    *tab,
		   XeditWindow *window)
{
	xedit_debug (DEBUG_COMMANDS);

	g_object_set_data (G_OBJECT (tab),
			   XEDIT_IS_CLOSING_TAB,
			   NULL);

	/* Trace tab state changes */
	g_signal_connect (tab,
			  "notify::state",
			  G_CALLBACK (tab_state_changed_while_saving),
			  window);

	xedit_window_set_active_tab (window, tab);
	file_save_as (tab, window);
}

static void
save_and_close_all_documents (const GList  *docs,
			      XeditWindow  *window)
{
	GList  *tabs;
	GList  *l;
	GSList *sl;
	GSList *tabs_to_save_as;
	GSList *tabs_to_save_and_close;
	GList  *tabs_to_close;

	xedit_debug (DEBUG_COMMANDS);

	g_return_if_fail (!(xedit_window_get_state (window) & XEDIT_WINDOW_STATE_PRINTING));

	tabs = gtk_container_get_children (
			GTK_CONTAINER (_xedit_window_get_notebook (window)));

	tabs_to_save_as = NULL;
	tabs_to_save_and_close = NULL;
	tabs_to_close = NULL;

	l = tabs;
	while (l != NULL)
	{
		XeditTab *t;
		XeditTabState state;
		XeditDocument *doc;

		t = XEDIT_TAB (l->data);

		state = xedit_tab_get_state (t);
		doc = xedit_tab_get_document (t);

		/* If the state is: ([*] invalid states)
		   - XEDIT_TAB_STATE_NORMAL: close (and if needed save)
		   - XEDIT_TAB_STATE_LOADING: close, we are sure the file is unmodified
		   - XEDIT_TAB_STATE_REVERTING: since the user wants
		     to return back to the version of the file she previously saved, we can close
		     without saving (CHECK: are we sure this is the right behavior, suppose the case 
		     the original file has been deleted)
		   - [*] XEDIT_TAB_STATE_SAVING: invalid, ClosAll
		     and Quit are unsensitive if the window state is SAVING.
		   - [*] XEDIT_TAB_STATE_PRINTING, XEDIT_TAB_STATE_PRINT_PREVIEWING: there is not a
		     real reason for not closing in this case, we do not save to avoid to run
		     two operations using the message area at the same time (may be we can remove
		     this limitation in the future). Note that ClosAll
		     and Quit are unsensitive if the window state is PRINTING.
		   - XEDIT_TAB_STATE_SHOWING_PRINT_PREVIEW: close (and if needed save)
		   - XEDIT_TAB_STATE_LOADING_ERROR: close without saving (if the state is LOADING_ERROR then the
		     document is not modified)
		   - XEDIT_TAB_STATE_REVERTING_ERROR: we do not close since the document contains errors
		   - XEDIT_TAB_STATE_SAVING_ERROR: we do not close since the document contains errors
		   - XEDIT_TAB_STATE_GENERIC_ERROR: we do not close since the document contains
		     errors (CHECK: we should problably remove this state)
		   - [*] XEDIT_TAB_STATE_CLOSING: this state is invalid in this case
		*/

		g_return_if_fail (state != XEDIT_TAB_STATE_PRINTING);
		g_return_if_fail (state != XEDIT_TAB_STATE_PRINT_PREVIEWING);
		g_return_if_fail (state != XEDIT_TAB_STATE_CLOSING);
		g_return_if_fail (state != XEDIT_TAB_STATE_SAVING);

		if ((state != XEDIT_TAB_STATE_SAVING_ERROR) &&
		    (state != XEDIT_TAB_STATE_GENERIC_ERROR) &&
		    (state != XEDIT_TAB_STATE_REVERTING_ERROR))
		{
			if ((g_list_index ((GList *)docs, doc) >= 0) &&
			    (state != XEDIT_TAB_STATE_LOADING) &&
			    (state != XEDIT_TAB_STATE_LOADING_ERROR) &&			    
			    (state != XEDIT_TAB_STATE_REVERTING)) /* CHECK: is this the right behavior with REVERTING ?*/
			{			
				/* The document must be saved before closing */
				g_return_if_fail (document_needs_saving (doc));
				
				/* FIXME: manage the case of local readonly files owned by the
				   user is running xedit - Paolo (Dec. 8, 2005) */
				if (xedit_document_is_untitled (doc) ||
				    xedit_document_get_readonly (doc))
				{
					g_object_set_data (G_OBJECT (t),
							   XEDIT_IS_CLOSING_TAB,
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
	xedit_window_close_tabs (window, tabs_to_close);
	g_list_free (tabs_to_close);

	/* Save and close all the files in tabs_to_save_and_close */
	sl = tabs_to_save_and_close;
	while (sl != NULL)
	{
		save_and_close (XEDIT_TAB (sl->data),
				window);
		sl = g_slist_next (sl);
	}
	g_slist_free (tabs_to_save_and_close);

	/* Save As and close all the files in tabs_to_save_as  */
	if (tabs_to_save_as != NULL)
	{
		XeditTab *tab;

		tabs_to_save_as = g_slist_reverse (tabs_to_save_as );

		g_return_if_fail (g_object_get_data (G_OBJECT (window),
						     XEDIT_LIST_OF_TABS_TO_SAVE_AS) == NULL);

		g_object_set_data (G_OBJECT (window),
				   XEDIT_LIST_OF_TABS_TO_SAVE_AS,
				   tabs_to_save_as);

		tab = XEDIT_TAB (tabs_to_save_as->data);

		save_as_and_close (tab, window);
	}
}

static void
save_and_close_document (const GList  *docs,
			 XeditWindow  *window)
{
	XeditTab *tab;

	xedit_debug (DEBUG_COMMANDS);

	g_return_if_fail (docs->next == NULL);

	tab = xedit_tab_get_from_document (XEDIT_DOCUMENT (docs->data));
	g_return_if_fail (tab != NULL);

	save_and_close (tab, window);
}

static void
close_all_tabs (XeditWindow *window)
{
	gboolean is_quitting;

	xedit_debug (DEBUG_COMMANDS);

	/* There is no document to save -> close all tabs */
	xedit_window_close_all_tabs (window);

	is_quitting = GPOINTER_TO_BOOLEAN (g_object_get_data (G_OBJECT (window),
							      XEDIT_IS_QUITTING));

	if (is_quitting)
		gtk_widget_destroy (GTK_WIDGET (window));

	return;
}

static void
close_document (XeditWindow   *window,
		XeditDocument *doc)
{
	XeditTab *tab;

	xedit_debug (DEBUG_COMMANDS);

	tab = xedit_tab_get_from_document (doc);
	g_return_if_fail (tab != NULL);

	xedit_window_close_tab (window, tab);
}

static void
close_confirmation_dialog_response_handler (XeditCloseConfirmationDialog *dlg,
					    gint                          response_id,
					    XeditWindow                  *window)
{
	GList *selected_documents;
	gboolean is_closing_all;

	xedit_debug (DEBUG_COMMANDS);

	is_closing_all = GPOINTER_TO_BOOLEAN (g_object_get_data (G_OBJECT (window),
					    			 XEDIT_IS_CLOSING_ALL));

	gtk_widget_hide (GTK_WIDGET (dlg));

	switch (response_id)
	{
		case GTK_RESPONSE_YES: /* Save and Close */
			selected_documents = xedit_close_confirmation_dialog_get_selected_documents (dlg);
			if (selected_documents == NULL)
			{
				if (is_closing_all)
				{
					/* There is no document to save -> close all tabs */
					/* We call gtk_widget_destroy before close_all_tabs
					 * because close_all_tabs could destroy the xedit window */
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
				 * because close_all_tabs could destroy the xedit window */
				gtk_widget_destroy (GTK_WIDGET (dlg));

				close_all_tabs (window);

				return;
			}
			else
			{
				const GList *unsaved_documents;

				unsaved_documents = xedit_close_confirmation_dialog_get_unsaved_documents (dlg);
				g_return_if_fail (unsaved_documents->next == NULL);

				close_document (window,
						XEDIT_DOCUMENT (unsaved_documents->data));
			}

			break;
		default: /* Do not close */

			/* Reset is_quitting flag */
			g_object_set_data (G_OBJECT (window),
					   XEDIT_IS_QUITTING,
					   GBOOLEAN_TO_POINTER (FALSE));

			break;
	}

	gtk_widget_destroy (GTK_WIDGET (dlg));
}

/* Returns TRUE if the tab can be immediately closed */
static gboolean
tab_can_close (XeditTab  *tab,
	       GtkWindow *window)
{
	XeditDocument *doc;

	xedit_debug (DEBUG_COMMANDS);

	doc = xedit_tab_get_document (tab);

	if (!_xedit_tab_can_close (tab))
	{
		GtkWidget     *dlg;

		dlg = xedit_close_confirmation_dialog_new_single (
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
 * xedit_window_close_tab always run the confirm dialog?
 * we should not allow closing a tab without resetting the
 * XEDIT_IS_CLOSING_ALL flag!
 */
void
_xedit_cmd_file_close_tab (XeditTab    *tab,
			   XeditWindow *window)
{
	xedit_debug (DEBUG_COMMANDS);

	g_return_if_fail (GTK_WIDGET (window) == gtk_widget_get_toplevel (GTK_WIDGET (tab)));

	g_object_set_data (G_OBJECT (window),
			   XEDIT_IS_CLOSING_ALL,
			   GBOOLEAN_TO_POINTER (FALSE));

	g_object_set_data (G_OBJECT (window),
			   XEDIT_IS_QUITTING,
			   GBOOLEAN_TO_POINTER (FALSE));

	g_object_set_data (G_OBJECT (window), 
	                   XEDIT_IS_QUITTING_ALL, 
	                   GINT_TO_POINTER (FALSE));


	if (tab_can_close (tab, GTK_WINDOW (window)))
		xedit_window_close_tab (window, tab);
}

void
_xedit_cmd_file_close (GtkAction   *action,
		      XeditWindow *window)
{
	XeditTab *active_tab;

	xedit_debug (DEBUG_COMMANDS);

	active_tab = xedit_window_get_active_tab (window);

	if (active_tab == NULL)
	{
		return;
	}

	_xedit_cmd_file_close_tab (active_tab, window);
}

/* Close all tabs */
static void
file_close_all (XeditWindow *window,
		gboolean     is_quitting)
{
	GList     *unsaved_docs;
	GtkWidget *dlg;

	xedit_debug (DEBUG_COMMANDS);

	g_return_if_fail (!(xedit_window_get_state (window) &
	                    (XEDIT_WINDOW_STATE_SAVING |
	                     XEDIT_WINDOW_STATE_PRINTING |
	                     XEDIT_WINDOW_STATE_SAVING_SESSION)));

	g_object_set_data (G_OBJECT (window),
			   XEDIT_IS_CLOSING_ALL,
			   GBOOLEAN_TO_POINTER (TRUE));

	g_object_set_data (G_OBJECT (window),
			   XEDIT_IS_QUITTING,
			   GBOOLEAN_TO_POINTER (is_quitting));
			   
	unsaved_docs = xedit_window_get_unsaved_documents (window);

	if (unsaved_docs == NULL)
	{
		/* There is no document to save -> close all tabs */
		xedit_window_close_all_tabs (window);

		if (is_quitting)
			gtk_widget_destroy (GTK_WIDGET (window));

		return;
	}

	if (unsaved_docs->next == NULL)
	{
		/* There is only one unsaved document */
		XeditTab      *tab;
		XeditDocument *doc;

		doc = XEDIT_DOCUMENT (unsaved_docs->data);

		tab = xedit_tab_get_from_document (doc);
		g_return_if_fail (tab != NULL);

		xedit_window_set_active_tab (window, tab);

		dlg = xedit_close_confirmation_dialog_new_single (
						GTK_WINDOW (window),
						doc,
						FALSE);
	}
	else
	{
		dlg = xedit_close_confirmation_dialog_new (GTK_WINDOW (window),
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
_xedit_cmd_file_close_all (GtkAction   *action,
			  XeditWindow *window)
{
	xedit_debug (DEBUG_COMMANDS);

	g_return_if_fail (!(xedit_window_get_state (window) &
	                    (XEDIT_WINDOW_STATE_SAVING |
	                    XEDIT_WINDOW_STATE_PRINTING |
	                    XEDIT_WINDOW_STATE_SAVING_SESSION)));

	file_close_all (window, FALSE);
}

void
_xedit_cmd_file_quit (GtkAction   *action,
		     XeditWindow *window)
{
	xedit_debug (DEBUG_COMMANDS);

	g_return_if_fail (!(xedit_window_get_state (window) &
	                    (XEDIT_WINDOW_STATE_SAVING |
	                     XEDIT_WINDOW_STATE_PRINTING |
	                     XEDIT_WINDOW_STATE_SAVING_SESSION)));

	file_close_all (window, TRUE);
}
