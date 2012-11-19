/*
 * pluma-help-commands.c
 * This file is part of pluma
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
 * list of people on the pluma Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "pluma-commands.h"
#include "pluma-debug.h"
#include "pluma-help.h"
#include "pluma-dirs.h"

void _pluma_cmd_help_contents(GtkAction* action, PlumaWindow* window)
{
	pluma_debug(DEBUG_COMMANDS);

	pluma_help_display(GTK_WINDOW(window), NULL, NULL);
}

void _pluma_cmd_help_about(GtkAction* action, PlumaWindow* window)
{
	static const gchar* const authors[] = {
		"Paolo Maggi <paolo@gnome.org>",
		"Paolo Borelli <pborelli@katamail.com>",
		"Steve Fr\303\251cinaux  <steve@istique.net>",
		"Jesse van den Kieboom  <jessevdk@gnome.org>",
		"Ignacio Casal Quinteiro <icq@gnome.org>",
		"James Willcox <jwillcox@gnome.org>",
		"Chema Celorio",
		"Federico Mena Quintero <federico@novell.com>",
		"Perberos <perberos@gmail.com>",
		NULL
	};

	static const gchar* const documenters[] = {
		"Sun GNOME Documentation Team <gdocteam@sun.com>",
		"Eric Baudais <baudais@okstate.edu>",
		NULL
	};

	static const gchar copyright[] = \
		"Copyright \xc2\xa9 1998-2000 Evan Lawrence, Alex Robert\n"
		"Copyright \xc2\xa9 2000-2002 Chema Celorio, Paolo Maggi\n"
		"Copyright \xc2\xa9 2003-2006 Paolo Maggi\n"
		"Copyright \xc2\xa9 2004-2010 Paolo Borelli, Jesse van den Kieboom\nSteve Fr\303\251cinaux, Ignacio Casal Quinteiro\n"
		"Copyright \xc2\xa9 2011 Perberos";

	static const gchar comments[] = \
		N_("pluma is a small and lightweight text editor for the MATE Desktop");

	pluma_debug (DEBUG_COMMANDS);

	gtk_show_about_dialog(GTK_WINDOW(window),
		"program-name", "Pluma",
		"authors", authors,
		"comments", _(comments),
		"copyright", copyright,
		"documenters", documenters,
		"logo_icon_name", "accessories-text-editor",
		"translator-credits", _("translator-credits"),
		"version", VERSION,
		"website", "http://mate-desktop.org",
		NULL);
}
