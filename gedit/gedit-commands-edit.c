/*
 * gedit-commands-edit.c
 * This file is part of gedit
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
 * Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the gedit Team, 1998-2005. See the AUTHORS file for a
 * list of people on the gedit Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

#include "gedit-commands.h"
#include "gedit-window.h"
#include "gedit-debug.h"
#include "gedit-view.h"
#include "dialogs/gedit-preferences-dialog.h"

void
_gedit_cmd_edit_undo (GtkAction   *action,
		     GeditWindow *window)
{
	GeditView *active_view;
	GtkSourceBuffer *active_document;

	gedit_debug (DEBUG_COMMANDS);

	active_view = gedit_window_get_active_view (window);
	g_return_if_fail (active_view);

	active_document = GTK_SOURCE_BUFFER (gtk_text_view_get_buffer (GTK_TEXT_VIEW (active_view)));

	gtk_source_buffer_undo (active_document);

	gedit_view_scroll_to_cursor (active_view);

	gtk_widget_grab_focus (GTK_WIDGET (active_view));
}

void
_gedit_cmd_edit_redo (GtkAction   *action,
		     GeditWindow *window)
{
	GeditView *active_view;
	GtkSourceBuffer *active_document;

	gedit_debug (DEBUG_COMMANDS);

	active_view = gedit_window_get_active_view (window);
	g_return_if_fail (active_view);

	active_document = GTK_SOURCE_BUFFER (gtk_text_view_get_buffer (GTK_TEXT_VIEW (active_view)));

	gtk_source_buffer_redo (active_document);

	gedit_view_scroll_to_cursor (active_view);

	gtk_widget_grab_focus (GTK_WIDGET (active_view));
}

void
_gedit_cmd_edit_cut (GtkAction   *action,
		    GeditWindow *window)
{
	GeditView *active_view;

	gedit_debug (DEBUG_COMMANDS);

	active_view = gedit_window_get_active_view (window);
	g_return_if_fail (active_view);

	gedit_view_cut_clipboard (active_view);

	gtk_widget_grab_focus (GTK_WIDGET (active_view));
}

void
_gedit_cmd_edit_copy (GtkAction   *action,
		     GeditWindow *window)
{
	GeditView *active_view;

	gedit_debug (DEBUG_COMMANDS);

	active_view = gedit_window_get_active_view (window);
	g_return_if_fail (active_view);

	gedit_view_copy_clipboard (active_view);

	gtk_widget_grab_focus (GTK_WIDGET (active_view));
}

void
_gedit_cmd_edit_paste (GtkAction   *action,
		      GeditWindow *window)
{
	GeditView *active_view;

	gedit_debug (DEBUG_COMMANDS);

	active_view = gedit_window_get_active_view (window);
	g_return_if_fail (active_view);

	gedit_view_paste_clipboard (active_view);

	gtk_widget_grab_focus (GTK_WIDGET (active_view));
}

void
_gedit_cmd_edit_delete (GtkAction   *action,
		       GeditWindow *window)
{
	GeditView *active_view;

	gedit_debug (DEBUG_COMMANDS);

	active_view = gedit_window_get_active_view (window);
	g_return_if_fail (active_view);

	gedit_view_delete_selection (active_view);

	gtk_widget_grab_focus (GTK_WIDGET (active_view));
}

void
_gedit_cmd_edit_select_all (GtkAction   *action,
			   GeditWindow *window)
{
	GeditView *active_view;

	gedit_debug (DEBUG_COMMANDS);

	active_view = gedit_window_get_active_view (window);
	g_return_if_fail (active_view);

	gedit_view_select_all (active_view);

	gtk_widget_grab_focus (GTK_WIDGET (active_view));
}

void
_gedit_cmd_edit_preferences (GtkAction   *action,
			    GeditWindow *window)
{
	gedit_debug (DEBUG_COMMANDS);

	gedit_show_preferences_dialog (window);
}
