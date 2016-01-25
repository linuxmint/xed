/*
 * xedit-documents-commands.c
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

#include <gtk/gtk.h>

#include "xedit-commands.h"
#include "xedit-window.h"
#include "xedit-notebook.h"
#include "xedit-debug.h"

void
_xedit_cmd_documents_previous_document (GtkAction   *action,
				       XeditWindow *window)
{
	GtkNotebook *notebook;

	xedit_debug (DEBUG_COMMANDS);

	notebook = GTK_NOTEBOOK (_xedit_window_get_notebook (window));
	gtk_notebook_prev_page (notebook);
}

void
_xedit_cmd_documents_next_document (GtkAction   *action,
				   XeditWindow *window)
{
	GtkNotebook *notebook;

	xedit_debug (DEBUG_COMMANDS);

	notebook = GTK_NOTEBOOK (_xedit_window_get_notebook (window));
	gtk_notebook_next_page (notebook);
}

void
_xedit_cmd_documents_move_to_new_window (GtkAction   *action,
					XeditWindow *window)
{
	XeditNotebook *old_notebook;
	XeditTab *tab;

	xedit_debug (DEBUG_COMMANDS);

	tab = xedit_window_get_active_tab (window);

	if (tab == NULL)
		return;

	old_notebook = XEDIT_NOTEBOOK (_xedit_window_get_notebook (window));

	g_return_if_fail (gtk_notebook_get_n_pages (GTK_NOTEBOOK (old_notebook)) > 1);

	_xedit_window_move_tab_to_new_window (window, tab);
}
