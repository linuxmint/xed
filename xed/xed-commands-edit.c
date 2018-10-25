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
