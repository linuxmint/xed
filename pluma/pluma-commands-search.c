/*
 * pluma-search-commands.c
 * This file is part of pluma
 *
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi
 * Copyright (C) 2002-2006 Paolo Maggi
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
 * Modified by the pluma Team, 1998-2006. See the AUTHORS file for a
 * list of people on the pluma Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#if GTK_CHECK_VERSION (3, 0, 0)
#include <gdk/gdkkeysyms-compat.h>
#define GTK_OBJECT G_OBJECT
#endif

#include "pluma-commands.h"
#include "pluma-debug.h"
#include "pluma-statusbar.h"
#include "pluma-window.h"
#include "pluma-window-private.h"
#include "pluma-utils.h"
#include "dialogs/pluma-search-dialog.h"

#define PLUMA_SEARCH_DIALOG_KEY		"pluma-search-dialog-key"
#define PLUMA_LAST_SEARCH_DATA_KEY	"pluma-last-search-data-key"

typedef struct _LastSearchData LastSearchData;
struct _LastSearchData
{
	gint x;
	gint y;
};

static void
last_search_data_free (LastSearchData *data)
{
	g_slice_free (LastSearchData, data);
}

static void
last_search_data_restore_position (PlumaSearchDialog *dlg)
{
	LastSearchData *data;

	data = g_object_get_data (G_OBJECT (dlg), PLUMA_LAST_SEARCH_DATA_KEY);

	if (data != NULL)
	{
		gtk_window_move (GTK_WINDOW (dlg),
				 data->x,
				 data->y);
	}
}

static void
last_search_data_store_position (PlumaSearchDialog *dlg)
{
	LastSearchData *data;
	
	data = g_object_get_data (G_OBJECT (dlg), PLUMA_LAST_SEARCH_DATA_KEY);
	
	if (data == NULL)
	{
		data = g_slice_new (LastSearchData);
		
		g_object_set_data_full (G_OBJECT (dlg),
					PLUMA_LAST_SEARCH_DATA_KEY,
					data,
					(GDestroyNotify) last_search_data_free);
	}
	
	gtk_window_get_position (GTK_WINDOW (dlg),
				 &data->x,
				 &data->y);
}

/* Use occurences only for Replace All */
static void
text_found (PlumaWindow *window,
	    gint         occurrences)
{
	if (occurrences > 1)
	{
		pluma_statusbar_flash_message (PLUMA_STATUSBAR (window->priv->statusbar),
					       window->priv->generic_message_cid,
					       ngettext("Found and replaced %d occurrence",
					     	        "Found and replaced %d occurrences",
					     	        occurrences),
					       occurrences);
	}
	else
	{
		if (occurrences == 1)
			pluma_statusbar_flash_message (PLUMA_STATUSBAR (window->priv->statusbar),
						       window->priv->generic_message_cid,
						       _("Found and replaced one occurrence"));
		else
			pluma_statusbar_flash_message (PLUMA_STATUSBAR (window->priv->statusbar),
						       window->priv->generic_message_cid,
						       " ");
	}
}

#define MAX_MSG_LENGTH 40
static void
text_not_found (PlumaWindow *window,
		const gchar *text)
{
	gchar *searched;
	
	searched = pluma_utils_str_end_truncate (text, MAX_MSG_LENGTH);

	pluma_statusbar_flash_message (PLUMA_STATUSBAR (window->priv->statusbar),
				       window->priv->generic_message_cid,
				       /* Translators: %s is replaced by the text
				          entered by the user in the search box */
				       _("\"%s\" not found"), searched);
	g_free (searched);
}

static gboolean
run_search (PlumaView   *view,
	    gboolean     wrap_around,
	    gboolean     search_backwards)
{
	PlumaDocument *doc;
	GtkTextIter start_iter;
	GtkTextIter match_start;
	GtkTextIter match_end;
	gboolean found = FALSE;

	doc = PLUMA_DOCUMENT (gtk_text_view_get_buffer (GTK_TEXT_VIEW (view)));

	if (!search_backwards)
	{
		gtk_text_buffer_get_selection_bounds (GTK_TEXT_BUFFER (doc),
						      NULL,
						      &start_iter);

		found = pluma_document_search_forward (doc,
						       &start_iter,
						       NULL,
						       &match_start,
						       &match_end);
	}
	else
	{
		gtk_text_buffer_get_selection_bounds (GTK_TEXT_BUFFER (doc),
						      &start_iter,
						      NULL);

		found = pluma_document_search_backward (doc,
						        NULL,
						        &start_iter,
						        &match_start,
						        &match_end);
	}

	if (!found && wrap_around)
	{
		if (!search_backwards)
			found = pluma_document_search_forward (doc,
							       NULL,
							       NULL, /* FIXME: set the end_inter */
							       &match_start,
							       &match_end);
		else
			found = pluma_document_search_backward (doc,
							        NULL, /* FIXME: set the start_inter */
							        NULL, 
							        &match_start,
							        &match_end);
	}
	
	if (found)
	{
		gtk_text_buffer_place_cursor (GTK_TEXT_BUFFER (doc),
					      &match_start);

		gtk_text_buffer_move_mark_by_name (GTK_TEXT_BUFFER (doc),
						   "selection_bound",
						   &match_end);

		pluma_view_scroll_to_cursor (view);
	}
	else
	{
		gtk_text_buffer_place_cursor (GTK_TEXT_BUFFER (doc),
					      &start_iter);
	}

	return found;
}

static void
do_find (PlumaSearchDialog *dialog,
	 PlumaWindow       *window)
{
	PlumaView *active_view;
	PlumaDocument *doc;
	gchar *search_text;
	const gchar *entry_text;
	gboolean match_case;
	gboolean entire_word;
	gboolean wrap_around;
	gboolean search_backwards;
	gboolean parse_escapes;
	guint flags = 0;
	guint old_flags = 0;
	gboolean found;

	/* TODO: make the dialog insensitive when all the tabs are closed
	 * and assert here that the view is not NULL */
	active_view = pluma_window_get_active_view (window);
	if (active_view == NULL)
		return;

	doc = PLUMA_DOCUMENT (gtk_text_view_get_buffer (GTK_TEXT_VIEW (active_view)));

	match_case = pluma_search_dialog_get_match_case (dialog);
	entire_word = pluma_search_dialog_get_entire_word (dialog);
	search_backwards = pluma_search_dialog_get_backwards (dialog);
	wrap_around = pluma_search_dialog_get_wrap_around (dialog);
	parse_escapes = pluma_search_dialog_get_parse_escapes (dialog);

	if (!parse_escapes) {
		entry_text = pluma_utils_escape_search_text (pluma_search_dialog_get_search_text (dialog));
	} else {
		entry_text = pluma_search_dialog_get_search_text (dialog);
	}

	PLUMA_SEARCH_SET_CASE_SENSITIVE (flags, match_case);
	PLUMA_SEARCH_SET_ENTIRE_WORD (flags, entire_word);

	search_text = pluma_document_get_search_text (doc, &old_flags);

	if ((search_text == NULL) ||
	    (strcmp (search_text, entry_text) != 0) ||
	    (flags != old_flags))
	{
		pluma_document_set_search_text (doc, entry_text, flags);
	}

	g_free (search_text);
	
	found = run_search (active_view,
			    wrap_around,
			    search_backwards);

	if (found)
		text_found (window, 0);
	else {
		if (!parse_escapes) {
			text_not_found (window, pluma_utils_unescape_search_text (entry_text));
		} else {
			text_not_found (window, entry_text);
		}
	}

	gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog),
					   PLUMA_SEARCH_DIALOG_REPLACE_RESPONSE,
					   found);
}

/* FIXME: move in pluma-document.c and share it with pluma-view */
static gboolean
get_selected_text (GtkTextBuffer  *doc,
		   gchar         **selected_text,
		   gint           *len)
{
	GtkTextIter start, end;

	g_return_val_if_fail (selected_text != NULL, FALSE);
	g_return_val_if_fail (*selected_text == NULL, FALSE);

	if (!gtk_text_buffer_get_selection_bounds (doc, &start, &end))
	{
		if (len != NULL)
			len = 0;

		return FALSE;
	}

	*selected_text = gtk_text_buffer_get_slice (doc, &start, &end, TRUE);

	if (len != NULL)
		*len = g_utf8_strlen (*selected_text, -1);

	return TRUE;
}

static void
replace_selected_text (GtkTextBuffer *buffer,
		       const gchar   *replace)
{
	g_return_if_fail (gtk_text_buffer_get_selection_bounds (buffer, NULL, NULL));
	g_return_if_fail (replace != NULL);

	gtk_text_buffer_begin_user_action (buffer);

	gtk_text_buffer_delete_selection (buffer, FALSE, TRUE);

	gtk_text_buffer_insert_at_cursor (buffer, replace, strlen (replace));

	gtk_text_buffer_end_user_action (buffer);
}

static void
do_replace (PlumaSearchDialog *dialog,
	    PlumaWindow       *window)
{
	PlumaDocument *doc;
	const gchar *search_entry_text;
	const gchar *replace_entry_text;
	gchar *unescaped_search_text;
	gchar *unescaped_replace_text;
	gchar *selected_text = NULL;
	gboolean match_case;
	gboolean parse_escapes;

	doc = pluma_window_get_active_document (window);
	if (doc == NULL)
		return;

	parse_escapes = pluma_search_dialog_get_parse_escapes (dialog);
	if (!parse_escapes) {
		search_entry_text = pluma_utils_escape_search_text (pluma_search_dialog_get_search_text (dialog));
	} else {
		search_entry_text = pluma_search_dialog_get_search_text (dialog);
	}
	g_return_if_fail ((search_entry_text) != NULL);
	g_return_if_fail ((*search_entry_text) != '\0');

	/* replace text may be "", we just delete */
	if (!parse_escapes) {
		replace_entry_text = pluma_utils_escape_search_text (pluma_search_dialog_get_replace_text (dialog));
	} else {
		replace_entry_text = pluma_search_dialog_get_replace_text (dialog);
	}
	g_return_if_fail ((replace_entry_text) != NULL);

	unescaped_search_text = pluma_utils_unescape_search_text (search_entry_text);

	get_selected_text (GTK_TEXT_BUFFER (doc), 
			   &selected_text, 
			   NULL);

	match_case = pluma_search_dialog_get_match_case (dialog);

	if ((selected_text == NULL) ||
	    (match_case && (strcmp (selected_text, unescaped_search_text) != 0)) || 
	    (!match_case && !g_utf8_caselessnmatch (selected_text,
						    unescaped_search_text, 
						    strlen (selected_text), 
						    strlen (unescaped_search_text)) != 0))
	{
		do_find (dialog, window);
		g_free (unescaped_search_text);
		g_free (selected_text);	

		return;
	}

	unescaped_replace_text = pluma_utils_unescape_search_text (replace_entry_text);	
	replace_selected_text (GTK_TEXT_BUFFER (doc), unescaped_replace_text);

	g_free (unescaped_search_text);
	g_free (selected_text);
	g_free (unescaped_replace_text);
	
	do_find (dialog, window);
}

static void
do_replace_all (PlumaSearchDialog *dialog,
		PlumaWindow       *window)
{
	PlumaView *active_view;
	PlumaDocument *doc;
	const gchar *search_entry_text;
	const gchar *replace_entry_text;
	gboolean match_case;
	gboolean entire_word;
	gboolean parse_escapes;
	guint flags = 0;
	gint count;

	active_view = pluma_window_get_active_view (window);
	if (active_view == NULL)
		return;

	doc = PLUMA_DOCUMENT (gtk_text_view_get_buffer (GTK_TEXT_VIEW (active_view)));

	parse_escapes = pluma_search_dialog_get_parse_escapes (dialog);
	if (!parse_escapes) {
		search_entry_text = pluma_utils_escape_search_text(pluma_search_dialog_get_search_text (dialog));
	} else {
		search_entry_text = pluma_search_dialog_get_search_text (dialog);
	}
	g_return_if_fail ((search_entry_text) != NULL);
	g_return_if_fail ((*search_entry_text) != '\0');

	/* replace text may be "", we just delete all occurrencies */
	if (!parse_escapes) {
		replace_entry_text = pluma_utils_escape_search_text (pluma_search_dialog_get_replace_text (dialog));
	} else {
		replace_entry_text = pluma_search_dialog_get_replace_text (dialog);
	}
	g_return_if_fail ((replace_entry_text) != NULL);

	match_case = pluma_search_dialog_get_match_case (dialog);
	entire_word = pluma_search_dialog_get_entire_word (dialog);

	PLUMA_SEARCH_SET_CASE_SENSITIVE (flags, match_case);
	PLUMA_SEARCH_SET_ENTIRE_WORD (flags, entire_word);

	count = pluma_document_replace_all (doc, 
					    search_entry_text,
					    replace_entry_text,
					    flags);

	if (count > 0)
	{
		text_found (window, count);
	}
	else
	{
		if (!parse_escapes) {
			text_not_found (window, pluma_utils_unescape_search_text (search_entry_text));
		} else {
			text_not_found (window, search_entry_text);
		}
	}

	gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog),
					   PLUMA_SEARCH_DIALOG_REPLACE_RESPONSE,
					   FALSE);
}

static void
search_dialog_response_cb (PlumaSearchDialog *dialog,
			   gint               response_id,
			   PlumaWindow       *window)
{
	pluma_debug (DEBUG_COMMANDS);

	switch (response_id)
	{
		case PLUMA_SEARCH_DIALOG_FIND_RESPONSE:
			do_find (dialog, window);
			break;
		case PLUMA_SEARCH_DIALOG_REPLACE_RESPONSE:
			do_replace (dialog, window);
			break;
		case PLUMA_SEARCH_DIALOG_REPLACE_ALL_RESPONSE:
			do_replace_all (dialog, window);
			break;
		default:
			last_search_data_store_position (dialog);
			gtk_widget_hide (GTK_WIDGET (dialog));
	}
}

static gboolean
search_dialog_delete_event_cb (GtkWidget   *widget,
			       GdkEventAny *event,
			       gpointer     user_data)
{
	pluma_debug (DEBUG_COMMANDS);

	/* prevent destruction */
	return TRUE;
}

static void
search_dialog_destroyed (PlumaWindow       *window,
			 PlumaSearchDialog *dialog)
{
	pluma_debug (DEBUG_COMMANDS);

	g_object_set_data (G_OBJECT (window),
			   PLUMA_SEARCH_DIALOG_KEY,
			   NULL);
	g_object_set_data (G_OBJECT (dialog),
			   PLUMA_LAST_SEARCH_DATA_KEY,
			   NULL);
}

static GtkWidget *
create_dialog (PlumaWindow *window, gboolean show_replace)
{
	GtkWidget *dialog;

	dialog = pluma_search_dialog_new (GTK_WINDOW (window), show_replace);

	g_signal_connect (dialog,
			  "response",
			  G_CALLBACK (search_dialog_response_cb),
			  window);
	g_signal_connect (dialog,
			 "delete-event",
			 G_CALLBACK (search_dialog_delete_event_cb),
			 NULL);

	g_object_set_data (G_OBJECT (window),
			   PLUMA_SEARCH_DIALOG_KEY,
			   dialog);

	g_object_weak_ref (G_OBJECT (dialog),
			   (GWeakNotify) search_dialog_destroyed,
			   window);

	return dialog;
}

void
_pluma_cmd_search_find (GtkAction   *action,
			PlumaWindow *window)
{
	gpointer data;
	GtkWidget *search_dialog;
	PlumaDocument *doc;
	gboolean selection_exists;
	gboolean parse_escapes;
	gchar *find_text = NULL;
	const gchar *search_text = NULL;
	gint sel_len;

	pluma_debug (DEBUG_COMMANDS);

	data = g_object_get_data (G_OBJECT (window), PLUMA_SEARCH_DIALOG_KEY);

	if (data == NULL)
	{
		search_dialog = create_dialog (window, FALSE);
	}
	else
	{
		g_return_if_fail (PLUMA_IS_SEARCH_DIALOG (data));
		
		search_dialog = GTK_WIDGET (data);
		
		/* turn the dialog into a find dialog if needed */
		if (pluma_search_dialog_get_show_replace (PLUMA_SEARCH_DIALOG (search_dialog)))
			pluma_search_dialog_set_show_replace (PLUMA_SEARCH_DIALOG (search_dialog),
							      FALSE);
	}

	doc = pluma_window_get_active_document (window);
	g_return_if_fail (doc != NULL);

	selection_exists = get_selected_text (GTK_TEXT_BUFFER (doc),
					      &find_text,
					      &sel_len);

	if (selection_exists && find_text != NULL && sel_len < 80)
	{
		/*
		 * Special case: if the currently selected text
		 * is the same as the unescaped search text and
		 * escape sequence parsing is activated, use the
		 * same old search text. (Without this, if you e.g.
		 * search for '\n' in escaped mode and then open
		 * the search dialog again, you'll get an unprintable
		 * single-character literal '\n' in the "search for"
		 * box).
		 */
		parse_escapes = pluma_search_dialog_get_parse_escapes (PLUMA_SEARCH_DIALOG (search_dialog));
		search_text = pluma_search_dialog_get_search_text (PLUMA_SEARCH_DIALOG (search_dialog));
		if (!(search_text != NULL
		      && !strcmp(pluma_utils_unescape_search_text(search_text), find_text)
		      && parse_escapes)) {
			/* General case */
			pluma_search_dialog_set_search_text (PLUMA_SEARCH_DIALOG (search_dialog),
							     find_text);
		}
		g_free (find_text);
	}
	else
	{
		g_free (find_text);
	}

	gtk_widget_show (search_dialog);
	last_search_data_restore_position (PLUMA_SEARCH_DIALOG (search_dialog));
	pluma_search_dialog_present_with_time (PLUMA_SEARCH_DIALOG (search_dialog),
					       GDK_CURRENT_TIME);
}

void
_pluma_cmd_search_replace (GtkAction   *action,
			   PlumaWindow *window)
{
	gpointer data;
	GtkWidget *replace_dialog;
	PlumaDocument *doc;
	gboolean selection_exists;
	gboolean parse_escapes;
	gchar *find_text = NULL;
	const gchar *search_text = NULL;
	gint sel_len;

	pluma_debug (DEBUG_COMMANDS);

	data = g_object_get_data (G_OBJECT (window), PLUMA_SEARCH_DIALOG_KEY);

	if (data == NULL)
	{
		replace_dialog = create_dialog (window, TRUE);
	}
	else
	{
		g_return_if_fail (PLUMA_IS_SEARCH_DIALOG (data));
		
		replace_dialog = GTK_WIDGET (data);
		
		/* turn the dialog into a find dialog if needed */
		if (!pluma_search_dialog_get_show_replace (PLUMA_SEARCH_DIALOG (replace_dialog)))
			pluma_search_dialog_set_show_replace (PLUMA_SEARCH_DIALOG (replace_dialog),
							      TRUE);
	}

	doc = pluma_window_get_active_document (window);
	g_return_if_fail (doc != NULL);

	selection_exists = get_selected_text (GTK_TEXT_BUFFER (doc),
					      &find_text,
					      &sel_len);

	if (selection_exists && find_text != NULL && sel_len < 80)
	{
		/*
		 * Special case: if the currently selected text
		 * is the same as the unescaped search text and
		 * escape sequence parsing is activated, use the
		 * same old search text. (Without this, if you e.g.
		 * search for '\n' in escaped mode and then open
		 * the search dialog again, you'll get an unprintable
		 * single-character literal '\n' in the "search for"
		 * box).
		 */
		parse_escapes = pluma_search_dialog_get_parse_escapes (PLUMA_SEARCH_DIALOG (replace_dialog));
		search_text = pluma_search_dialog_get_search_text (PLUMA_SEARCH_DIALOG (replace_dialog));
		if (!(search_text != NULL
		      && !strcmp(pluma_utils_unescape_search_text(search_text), find_text)
		      && parse_escapes)) {
			/* General case */
			pluma_search_dialog_set_search_text (PLUMA_SEARCH_DIALOG (replace_dialog),
							     find_text);
		}
		g_free (find_text);
	}
	else
	{
		g_free (find_text);
	}

	gtk_widget_show (replace_dialog);
	last_search_data_restore_position (PLUMA_SEARCH_DIALOG (replace_dialog));
	pluma_search_dialog_present_with_time (PLUMA_SEARCH_DIALOG (replace_dialog),
					       GDK_CURRENT_TIME);
}

static void
do_find_again (PlumaWindow *window,
	       gboolean     backward)
{
	PlumaView *active_view;
	gboolean wrap_around = TRUE;
	gpointer data;
	
	active_view = pluma_window_get_active_view (window);
	g_return_if_fail (active_view != NULL);

	data = g_object_get_data (G_OBJECT (window), PLUMA_SEARCH_DIALOG_KEY);
	
	if (data != NULL)
		wrap_around = pluma_search_dialog_get_wrap_around (PLUMA_SEARCH_DIALOG (data));
	
	run_search (active_view,
		    wrap_around,
		    backward);
}

void
_pluma_cmd_search_find_next (GtkAction   *action,
			     PlumaWindow *window)
{
	pluma_debug (DEBUG_COMMANDS);

	do_find_again (window, FALSE);
}

void
_pluma_cmd_search_find_prev (GtkAction   *action,
			     PlumaWindow *window)
{
	pluma_debug (DEBUG_COMMANDS);

	do_find_again (window, TRUE);
}

void
_pluma_cmd_search_clear_highlight (GtkAction   *action,
				   PlumaWindow *window)
{
	PlumaDocument *doc;

	pluma_debug (DEBUG_COMMANDS);

	doc = pluma_window_get_active_document (window);
	pluma_document_set_search_text (PLUMA_DOCUMENT (doc),
					"",
					PLUMA_SEARCH_DONT_SET_FLAGS);
}

void
_pluma_cmd_search_goto_line (GtkAction   *action,
			     PlumaWindow *window)
{
	PlumaView *active_view;

	pluma_debug (DEBUG_COMMANDS);

	active_view = pluma_window_get_active_view (window);
	if (active_view == NULL)
		return;

	/* Focus the view if needed: we need to focus the view otherwise 
	   activating the binding for goto line has no effect */
	gtk_widget_grab_focus (GTK_WIDGET (active_view));


	/* goto line is builtin in PlumaView, just activate
	 * the corrisponding binding.
	 */
	gtk_bindings_activate (GTK_OBJECT (active_view),
			       GDK_i,
			       GDK_CONTROL_MASK);
}

void
_pluma_cmd_search_incremental_search (GtkAction   *action,
				      PlumaWindow *window)
{
	PlumaView *active_view;

	pluma_debug (DEBUG_COMMANDS);

	active_view = pluma_window_get_active_view (window);
	if (active_view == NULL)
		return;

	/* Focus the view if needed: we need to focus the view otherwise 
	   activating the binding for incremental search has no effect */
	gtk_widget_grab_focus (GTK_WIDGET (active_view));
	
	/* incremental search is builtin in PlumaView, just activate
	 * the corrisponding binding.
	 */
	gtk_bindings_activate (GTK_OBJECT (active_view),
			       GDK_k,
			       GDK_CONTROL_MASK);
}
