/*
 * xed-documents-commands.c
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
#include "xed-notebook.h"
#include "xed-debug.h"

void
_xed_cmd_documents_previous_document (GtkAction   *action,
				       XedWindow *window)
{
	GtkNotebook *notebook;

	xed_debug (DEBUG_COMMANDS);

	notebook = GTK_NOTEBOOK (_xed_window_get_notebook (window));
	gtk_notebook_prev_page (notebook);
}

void
_xed_cmd_documents_next_document (GtkAction   *action,
				   XedWindow *window)
{
	GtkNotebook *notebook;

	xed_debug (DEBUG_COMMANDS);

	notebook = GTK_NOTEBOOK (_xed_window_get_notebook (window));
	gtk_notebook_next_page (notebook);
}

void
_xed_cmd_documents_move_to_new_window (GtkAction   *action,
					XedWindow *window)
{
	XedNotebook *old_notebook;
	XedTab *tab;

	xed_debug (DEBUG_COMMANDS);

	tab = xed_window_get_active_tab (window);

	if (tab == NULL)
		return;

	old_notebook = XED_NOTEBOOK (_xed_window_get_notebook (window));

	g_return_if_fail (gtk_notebook_get_n_pages (GTK_NOTEBOOK (old_notebook)) > 1);

	_xed_window_move_tab_to_new_window (window, tab);
}
