/*
 * xed-commands-edit.c
 * This file is part of xed
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
 * Modified by the xed Team, 1998-2005. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#include <config.h>
#include <gtk/gtk.h>

#include "xed-commands.h"
#include "xed-window.h"
#include "xed-debug.h"
#include "xed-view.h"
#include "xed-preferences-dialog.h"

void
_xed_cmd_edit_undo (GtkAction   *action,
		     XedWindow *window)
{
	XedView *active_view;
	GtkSourceBuffer *active_document;

	xed_debug (DEBUG_COMMANDS);

	active_view = xed_window_get_active_view (window);
	g_return_if_fail (active_view);

	active_document = GTK_SOURCE_BUFFER (gtk_text_view_get_buffer (GTK_TEXT_VIEW (active_view)));

	gtk_source_buffer_undo (active_document);

	xed_view_scroll_to_cursor (active_view);

	gtk_widget_grab_focus (GTK_WIDGET (active_view));
}

void
_xed_cmd_edit_redo (GtkAction   *action,
		     XedWindow *window)
{
	XedView *active_view;
	GtkSourceBuffer *active_document;

	xed_debug (DEBUG_COMMANDS);

	active_view = xed_window_get_active_view (window);
	g_return_if_fail (active_view);

	active_document = GTK_SOURCE_BUFFER (gtk_text_view_get_buffer (GTK_TEXT_VIEW (active_view)));

	gtk_source_buffer_redo (active_document);

	xed_view_scroll_to_cursor (active_view);

	gtk_widget_grab_focus (GTK_WIDGET (active_view));
}

void
_xed_cmd_edit_cut (GtkAction   *action,
		    XedWindow *window)
{
	XedView *active_view;

	xed_debug (DEBUG_COMMANDS);

	active_view = xed_window_get_active_view (window);
	g_return_if_fail (active_view);

	xed_view_cut_clipboard (active_view);

	gtk_widget_grab_focus (GTK_WIDGET (active_view));
}

void
_xed_cmd_edit_copy (GtkAction   *action,
		     XedWindow *window)
{
	XedView *active_view;

	xed_debug (DEBUG_COMMANDS);

	active_view = xed_window_get_active_view (window);
	g_return_if_fail (active_view);

	xed_view_copy_clipboard (active_view);

	gtk_widget_grab_focus (GTK_WIDGET (active_view));
}

void
_xed_cmd_edit_paste (GtkAction   *action,
		      XedWindow *window)
{
	XedView *active_view;

	xed_debug (DEBUG_COMMANDS);

	active_view = xed_window_get_active_view (window);
	g_return_if_fail (active_view);

	xed_view_paste_clipboard (active_view);

	gtk_widget_grab_focus (GTK_WIDGET (active_view));
}

void
_xed_cmd_edit_delete (GtkAction   *action,
		       XedWindow *window)
{
	XedView *active_view;

	xed_debug (DEBUG_COMMANDS);

	active_view = xed_window_get_active_view (window);
	g_return_if_fail (active_view);

	xed_view_delete_selection (active_view);

	gtk_widget_grab_focus (GTK_WIDGET (active_view));
}

void
_xed_cmd_edit_duplicate (GtkAction   *action,
		     XedWindow *window)
{
	XedView *active_view;

	xed_debug (DEBUG_COMMANDS);

	active_view = xed_window_get_active_view (window);
	g_return_if_fail (active_view);

	xed_view_duplicate (active_view);

	gtk_widget_grab_focus (GTK_WIDGET (active_view));
}

void
_xed_cmd_edit_select_all (GtkAction   *action,
			   XedWindow *window)
{
	XedView *active_view;

	xed_debug (DEBUG_COMMANDS);

	active_view = xed_window_get_active_view (window);
	g_return_if_fail (active_view);

	xed_view_select_all (active_view);

	gtk_widget_grab_focus (GTK_WIDGET (active_view));
}

void
_xed_cmd_edit_preferences (GtkAction   *action,
			    XedWindow *window)
{
	xed_debug (DEBUG_COMMANDS);

	xed_show_preferences_dialog (window);
}

void
_xed_cmd_edit_toggle_comment (GtkAction *action,
                              XedWindow *window)
{
    XedView *active_view;
    GtkSourceBuffer *active_document;
    GtkSourceLanguage *language;
    const gchar *comment_text;
    gint start_line;
    gint end_line;
    gint i;
    gboolean is_comment = FALSE;
    GtkTextIter start_iter;
    GtkTextIter end_iter;

    xed_debug (DEBUG_COMMANDS);

    active_view = xed_window_get_active_view (window);

    if (active_view == NULL)
    {
        return;
    }

    active_document = GTK_SOURCE_BUFFER (gtk_text_view_get_buffer (GTK_TEXT_VIEW (active_view)));
    language = gtk_source_buffer_get_language (active_document);

    if (language == NULL)
    {
        return;
    }

    comment_text = gtk_source_language_get_metadata (language, "line-comment-start");

    if (comment_text == NULL)
    {
        return;
    }

    gtk_text_buffer_get_selection_bounds (GTK_TEXT_BUFFER (active_document), &start_iter, &end_iter);
    start_line = gtk_text_iter_get_line (&start_iter);
    end_line = gtk_text_iter_get_line (&end_iter);

    gtk_text_buffer_begin_user_action (GTK_TEXT_BUFFER (active_document)); // begin

    // if some lines are already commented, consider the whole block commented and uncomment them
    for (i = start_line; i <= end_line; i++)
    {
        GtkTextIter start_line_iter;
        GtkTextIter end_line_iter;
        const gchar *line_text;

        gtk_text_buffer_get_iter_at_line (GTK_TEXT_BUFFER (active_document), &start_line_iter, i);
        end_line_iter = start_line_iter;
        gtk_text_iter_forward_to_line_end (&end_line_iter);

        line_text = gtk_text_buffer_get_slice (GTK_TEXT_BUFFER (active_document), &start_line_iter, &end_line_iter, TRUE);
        if (g_str_has_prefix (line_text, comment_text))
        {
            is_comment = TRUE;
            end_line_iter = start_line_iter;
            gtk_text_iter_forward_chars (&end_line_iter, strlen(comment_text));
            gtk_text_buffer_delete (GTK_TEXT_BUFFER (active_document), &start_line_iter, &end_line_iter);
        }
    }

    // only comment if nothing was commented to begin with
    if (!is_comment)
    {
        for (i = start_line; i <= end_line; i++)
        {
            GtkTextIter insert_iter;

            gtk_text_buffer_get_iter_at_line (GTK_TEXT_BUFFER (active_document), &insert_iter, i);
            gtk_text_buffer_insert (GTK_TEXT_BUFFER (active_document), &insert_iter, comment_text, -1);
        }
    }

    gtk_text_buffer_end_user_action (GTK_TEXT_BUFFER (active_document)); // end
}

void
_xed_cmd_edit_toggle_comment_block (GtkAction *action,
                                    XedWindow *window)
{
    XedView *active_view;
    GtkSourceBuffer *active_document;
    GtkSourceLanguage *language;
    const gchar *start_text;
    const gchar *end_text;
    GtkTextIter start_iter;
    GtkTextIter end_iter;
    gchar *selected_text;
    gchar *insert_text;

    xed_debug (DEBUG_COMMANDS);

    active_view = xed_window_get_active_view (window);

    if (active_view == NULL)
    {
        return;
    }

    active_document = GTK_SOURCE_BUFFER (gtk_text_view_get_buffer (GTK_TEXT_VIEW (active_view)));
    language = gtk_source_buffer_get_language (active_document);

    if (language == NULL)
    {
        return;
    }

    start_text = gtk_source_language_get_metadata (language, "block-comment-start");
    end_text = gtk_source_language_get_metadata (language, "block-comment-end");

    if (start_text == NULL || end_text == NULL)
    {
        return;
    }

    gtk_text_buffer_get_selection_bounds (GTK_TEXT_BUFFER (active_document), &start_iter, &end_iter);

    selected_text = gtk_text_buffer_get_slice (GTK_TEXT_BUFFER (active_document), &start_iter, &end_iter, TRUE);

    if (g_str_has_prefix (selected_text, start_text) && g_str_has_suffix (selected_text, end_text))
    {
        gint start = strlen (start_text);
        gint end = strlen (end_text);
        const gchar *tmp = selected_text + start;
        insert_text = g_strndup (tmp , strlen(selected_text) - start - end);
    }
    else
    {
        insert_text = g_strconcat (start_text, selected_text, end_text, NULL);
    }

    gtk_text_buffer_begin_user_action (GTK_TEXT_BUFFER (active_document)); // begin

    // replace the selected text with the commented/uncommented version
    gtk_text_buffer_delete (GTK_TEXT_BUFFER (active_document), &start_iter, &end_iter);
    gtk_text_buffer_insert (GTK_TEXT_BUFFER (active_document), &end_iter, insert_text, -1);

    // move selection back where it was
    gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (active_document), &start_iter,
                                        gtk_text_iter_get_offset (&end_iter) - strlen (insert_text));
    gtk_text_buffer_select_range (GTK_TEXT_BUFFER (active_document), &start_iter, &end_iter);

    gtk_text_buffer_end_user_action (GTK_TEXT_BUFFER (active_document)); // end

    g_free (selected_text);
    g_free (insert_text);
}
