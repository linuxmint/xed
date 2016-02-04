/*
 * xed-help-commands.c
 * This file is part of xed
 *
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi
 * Copyright (C) 2002-2005 Paolo Maggi
 * Copyright (C) 2011 Perberos
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
 * Modified by the gedit Team, 1998-2005. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "xed-commands.h"
#include "xed-debug.h"
#include "xed-help.h"
#include "xed-dirs.h"

void _xed_cmd_help_contents(GtkAction* action, XedWindow* window)
{
	xed_debug(DEBUG_COMMANDS);

	xed_help_display(GTK_WINDOW(window), NULL, NULL);
}

void _xed_cmd_help_about(GtkAction* action, XedWindow* window)
{
	static const gchar comments[] = \
		N_("A small and lightweight text editor");

	xed_debug (DEBUG_COMMANDS);

	gtk_show_about_dialog(GTK_WINDOW(window),
		"program-name", "xed",
		"comments", _(comments),
		"logo_icon_name", "accessories-text-editor",
		"version", VERSION,
		"website", "http://github.com/linuxmint/xed",
		NULL);
}
