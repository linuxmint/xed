/*
 * gedit-help.c
 * This file is part of gedit
 *
 * Copyright (C) 2005 - Paolo Maggi 
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
 * Modified by the gedit Team, 2005. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gedit-help.h"

#include <glib/gi18n.h>
#include <string.h>
#include <gtk/gtk.h>

#ifdef OS_OSX
#include "osx/gedit-osx.h"
#endif

gboolean    
gedit_help_display (GtkWindow   *parent,
		    const gchar *name, /* "gedit" if NULL */
		    const gchar *link_id)
{
	GError *error = NULL;
	gboolean ret;
	gchar *link;
	
	g_return_val_if_fail ((parent == NULL) || GTK_IS_WINDOW (parent), FALSE);

#ifdef OS_OSX
	if (name == NULL || strcmp(name, "gedit.xml") == NULL || strcmp(name, "gedit") == 0)
	{
		return gedit_osx_show_help (link_id);
	}
	else
	{
		return FALSE;
	}
#endif

	if (name == NULL)
		name = "gedit";
	else if (strcmp (name, "gedit.xml") == 0)
	{
		g_warning ("%s: Using \"gedit.xml\" for the help name is deprecated, use \"gedit\" or simply NULL instead", G_STRFUNC);
		
		name = "gedit";
	}

#ifndef G_OS_WIN32
	if (link_id)
		link = g_strdup_printf ("ghelp:%s?%s", name, link_id);
	else
		link = g_strdup_printf ("ghelp:%s", name);
#else
	if (link_id)
		link = g_strdup_printf ("http://library.mate.org/users/gedit/stable/%s",
					link_id);
	else
		link = g_strdup ("http://library.mate.org/users/gedit/stable/");
#endif

	ret = gtk_show_uri (gtk_widget_get_screen (GTK_WIDGET (parent)),
	                    link, 
			    GDK_CURRENT_TIME, 
			    &error);

	g_free (link);

	if (error != NULL)
	{
		GtkWidget *dialog;

		dialog = gtk_message_dialog_new (parent,
						 GTK_DIALOG_DESTROY_WITH_PARENT,
						 GTK_MESSAGE_ERROR,
						 GTK_BUTTONS_CLOSE, 
						 _("There was an error displaying the help."));

		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
							  "%s", error->message);

		g_signal_connect (G_OBJECT (dialog),
				  "response",
				  G_CALLBACK (gtk_widget_destroy),
				  NULL);

		gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

		gtk_widget_show (dialog);

		g_error_free (error);
	}

	return ret;
}
