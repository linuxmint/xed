/*
 * gedit-commands-file-print.c
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

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "gedit-commands.h"
#include "gedit-window.h"
#include "gedit-tab.h"
#include "gedit-debug.h"

#if !GTK_CHECK_VERSION (2, 17, 4)
void
_gedit_cmd_file_page_setup (GtkAction   *action,
			    GeditWindow *window)
{
	GeditTab *tab;

	gedit_debug (DEBUG_COMMANDS);

	tab = gedit_window_get_active_tab (window);
	if (tab == NULL)
		return;

	_gedit_tab_page_setup (tab);
}
#endif

void
_gedit_cmd_file_print_preview (GtkAction   *action,
			       GeditWindow *window)
{
	GeditTab *tab;

	gedit_debug (DEBUG_COMMANDS);

	tab = gedit_window_get_active_tab (window);
	if (tab == NULL)
		return;

	_gedit_tab_print_preview (tab);
}

void
_gedit_cmd_file_print (GtkAction   *action,
		       GeditWindow *window)
{
	GeditTab *tab;

	gedit_debug (DEBUG_COMMANDS);

	tab = gedit_window_get_active_tab (window);
	if (tab == NULL)
		return;

	_gedit_tab_print (tab);
}

