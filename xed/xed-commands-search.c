/*
 * xed-search-commands.c
 * This file is part of xed
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
 * Modified by the xed Team, 1998-2006. See the AUTHORS file for a
 * list of people on the xed Team.
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

#include "xed-commands.h"
#include "xed-debug.h"
#include "xed-window.h"
#include "xed-utils.h"
#include "xed-searchbar.h"


void
_xed_cmd_search_find (GtkAction   *action,
			XedWindow *window)
{
	xed_searchbar_show (xed_window_get_searchbar (window), FALSE);
}

void
_xed_cmd_search_replace (GtkAction   *action,
			   XedWindow *window)
{
	xed_searchbar_show (xed_window_get_searchbar (window), TRUE);
}

void
_xed_cmd_search_find_next (GtkAction   *action,
			     XedWindow *window)
{
	xed_debug (DEBUG_COMMANDS);

	xed_searchbar_find_again (xed_window_get_searchbar (window), FALSE);
}

void
_xed_cmd_search_find_prev (GtkAction   *action,
			     XedWindow *window)
{
	xed_debug (DEBUG_COMMANDS);

	xed_searchbar_find_again (xed_window_get_searchbar (window), TRUE);
}

void
_xed_cmd_search_clear_highlight (GtkAction   *action,
				   XedWindow *window)
{
	XedDocument *doc;

	xed_debug (DEBUG_COMMANDS);

	doc = xed_window_get_active_document (window);
	xed_document_set_search_text (XED_DOCUMENT (doc),
					"",
					XED_SEARCH_DONT_SET_FLAGS);
}

void
_xed_cmd_search_goto_line (GtkAction   *action,
			     XedWindow *window)
{
	XedView *active_view;

	xed_debug (DEBUG_COMMANDS);

	active_view = xed_window_get_active_view (window);
	if (active_view == NULL)
		return;

	/* Focus the view if needed: we need to focus the view otherwise 
	   activating the binding for goto line has no effect */
	gtk_widget_grab_focus (GTK_WIDGET (active_view));


	/* goto line is builtin in XedView, just activate
	 * the corrisponding binding.
	 */
	gtk_bindings_activate (G_OBJECT (active_view),
			       GDK_KEY_i,
			       GDK_CONTROL_MASK);
}

void
_xed_cmd_search_incremental_search (GtkAction   *action,
				      XedWindow *window)
{
	XedView *active_view;

	xed_debug (DEBUG_COMMANDS);

	active_view = xed_window_get_active_view (window);
	if (active_view == NULL)
		return;

	/* Focus the view if needed: we need to focus the view otherwise 
	   activating the binding for incremental search has no effect */
	gtk_widget_grab_focus (GTK_WIDGET (active_view));
	
	/* incremental search is builtin in XedView, just activate
	 * the corrisponding binding.
	 */
	gtk_bindings_activate (G_OBJECT (active_view),
			       GDK_KEY_k,
			       GDK_CONTROL_MASK);
}
