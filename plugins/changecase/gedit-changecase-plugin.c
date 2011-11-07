/*
 * gedit-changecase-plugin.c
 * 
 * Copyright (C) 2004-2005 - Paolo Borelli
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gedit-changecase-plugin.h"

#include <glib/gi18n-lib.h>
#include <gmodule.h>

#include <gedit/gedit-debug.h>

#define WINDOW_DATA_KEY "GeditChangecasePluginWindowData"

GEDIT_PLUGIN_REGISTER_TYPE(GeditChangecasePlugin, gedit_changecase_plugin)

typedef enum {
	TO_UPPER_CASE,
	TO_LOWER_CASE,
	INVERT_CASE,
	TO_TITLE_CASE,
} ChangeCaseChoice;

static void
do_upper_case (GtkTextBuffer *buffer,
               GtkTextIter   *start,
               GtkTextIter   *end)
{
	GString *s = g_string_new (NULL);

	while (!gtk_text_iter_is_end (start) &&
	       !gtk_text_iter_equal (start, end))
	{
		gunichar c, nc;

		c = gtk_text_iter_get_char (start);
		nc = g_unichar_toupper (c);
		g_string_append_unichar (s, nc);

		gtk_text_iter_forward_char (start);
	}

	gtk_text_buffer_delete_selection (buffer, TRUE, TRUE);
	gtk_text_buffer_insert_at_cursor (buffer, s->str, s->len);

	g_string_free (s, TRUE);
}

static void
do_lower_case (GtkTextBuffer *buffer,
               GtkTextIter   *start,
               GtkTextIter   *end)
{
	GString *s = g_string_new (NULL);

	while (!gtk_text_iter_is_end (start) &&
	       !gtk_text_iter_equal (start, end))
	{
		gunichar c, nc;

		c = gtk_text_iter_get_char (start);
		nc = g_unichar_tolower (c);
		g_string_append_unichar (s, nc);

		gtk_text_iter_forward_char (start);
	}

	gtk_text_buffer_delete_selection (buffer, TRUE, TRUE);
	gtk_text_buffer_insert_at_cursor (buffer, s->str, s->len);

	g_string_free (s, TRUE);
}

static void
do_invert_case (GtkTextBuffer *buffer,
                GtkTextIter   *start,
                GtkTextIter   *end)
{
	GString *s = g_string_new (NULL);

	while (!gtk_text_iter_is_end (start) &&
	       !gtk_text_iter_equal (start, end))
	{
		gunichar c, nc;

		c = gtk_text_iter_get_char (start);
		if (g_unichar_islower (c))
			nc = g_unichar_toupper (c);
		else
			nc = g_unichar_tolower (c);
		g_string_append_unichar (s, nc);

		gtk_text_iter_forward_char (start);
	}

	gtk_text_buffer_delete_selection (buffer, TRUE, TRUE);
	gtk_text_buffer_insert_at_cursor (buffer, s->str, s->len);

	g_string_free (s, TRUE);
}

static void
do_title_case (GtkTextBuffer *buffer,
               GtkTextIter   *start,
               GtkTextIter   *end)
{
	GString *s = g_string_new (NULL);

	while (!gtk_text_iter_is_end (start) &&
	       !gtk_text_iter_equal (start, end))
	{
		gunichar c, nc;

		c = gtk_text_iter_get_char (start);
		if (gtk_text_iter_starts_word (start))
			nc = g_unichar_totitle (c);
		else
			nc = g_unichar_tolower (c);
		g_string_append_unichar (s, nc);

		gtk_text_iter_forward_char (start);
	}

	gtk_text_buffer_delete_selection (buffer, TRUE, TRUE);
	gtk_text_buffer_insert_at_cursor (buffer, s->str, s->len);

	g_string_free (s, TRUE);
}

static void
change_case (GeditWindow      *window,
             ChangeCaseChoice  choice)
{
	GeditDocument *doc;
	GtkTextIter start, end;

	gedit_debug (DEBUG_PLUGINS);

	doc = gedit_window_get_active_document (window);
	g_return_if_fail (doc != NULL);

	if (!gtk_text_buffer_get_selection_bounds (GTK_TEXT_BUFFER (doc),
						   &start, &end))
	{
		return;
	}

	gtk_text_buffer_begin_user_action (GTK_TEXT_BUFFER (doc));

	switch (choice)
	{
	case TO_UPPER_CASE:
		do_upper_case (GTK_TEXT_BUFFER (doc), &start, &end);
		break;
	case TO_LOWER_CASE:
		do_lower_case (GTK_TEXT_BUFFER (doc), &start, &end);
		break;
	case INVERT_CASE:
		do_invert_case (GTK_TEXT_BUFFER (doc), &start, &end);
		break;
	case TO_TITLE_CASE:
		do_title_case (GTK_TEXT_BUFFER (doc), &start, &end);
		break;
	default:
		g_return_if_reached ();
	}

	gtk_text_buffer_end_user_action (GTK_TEXT_BUFFER (doc));
}

static void
upper_case_cb (GtkAction   *action,
               GeditWindow *window)
{
	change_case (window, TO_UPPER_CASE);
}

static void
lower_case_cb (GtkAction   *action,
               GeditWindow *window)
{
	change_case (window, TO_LOWER_CASE);
}

static void
invert_case_cb (GtkAction   *action,
                GeditWindow *window)
{
	change_case (window, INVERT_CASE);
}

static void
title_case_cb (GtkAction   *action,
               GeditWindow *window)
{
	change_case (window, TO_TITLE_CASE);
}

static const GtkActionEntry action_entries[] =
{
	{ "ChangeCase", NULL, N_("C_hange Case") },
	{ "UpperCase", NULL, N_("All _Upper Case"), NULL,
	  N_("Change selected text to upper case"),
	  G_CALLBACK (upper_case_cb) },
	{ "LowerCase", NULL, N_("All _Lower Case"), NULL,
	  N_("Change selected text to lower case"),
	  G_CALLBACK (lower_case_cb) },
	{ "InvertCase", NULL, N_("_Invert Case"), NULL,
	  N_("Invert the case of selected text"),
	  G_CALLBACK (invert_case_cb) },
	{ "TitleCase", NULL, N_("_Title Case"), NULL,
	  N_("Capitalize the first letter of each selected word"),
	  G_CALLBACK (title_case_cb) }
};

const gchar submenu[] =
"<ui>"
"  <menubar name='MenuBar'>"
"    <menu name='EditMenu' action='Edit'>"
"      <placeholder name='EditOps_6'>"
"        <menu action='ChangeCase'>"
"          <menuitem action='UpperCase'/>"
"          <menuitem action='LowerCase'/>"
"          <menuitem action='InvertCase'/>"
"          <menuitem action='TitleCase'/>"
"        </menu>"
"      </placeholder>"
"    </menu>"
"  </menubar>"
"</ui>";

static void
gedit_changecase_plugin_init (GeditChangecasePlugin *plugin)
{
	gedit_debug_message (DEBUG_PLUGINS, "GeditChangecasePlugin initializing");
}

static void
gedit_changecase_plugin_finalize (GObject *object)
{
	G_OBJECT_CLASS (gedit_changecase_plugin_parent_class)->finalize (object);

	gedit_debug_message (DEBUG_PLUGINS, "GeditChangecasePlugin finalizing");
}

typedef struct
{
	GtkActionGroup *action_group;
	guint           ui_id;
} WindowData;

static void
free_window_data (WindowData *data)
{
	g_return_if_fail (data != NULL);

	g_slice_free (WindowData, data);
}

static void
update_ui_real (GeditWindow  *window,
		WindowData   *data)
{
	GtkTextView *view;
	GtkAction *action;
	gboolean sensitive = FALSE;

	gedit_debug (DEBUG_PLUGINS);

	view = GTK_TEXT_VIEW (gedit_window_get_active_view (window));

	if (view != NULL)
	{
		GtkTextBuffer *buffer;

		buffer = gtk_text_view_get_buffer (view);
		sensitive = (gtk_text_view_get_editable (view) &&
			     gtk_text_buffer_get_has_selection (buffer));
	}

	action = gtk_action_group_get_action (data->action_group,
					      "ChangeCase");
	gtk_action_set_sensitive (action, sensitive);
}

static void
impl_activate (GeditPlugin *plugin,
	       GeditWindow *window)
{
	GtkUIManager *manager;
	WindowData *data;
	GError *error = NULL;

	gedit_debug (DEBUG_PLUGINS);

	data = g_slice_new (WindowData);

	manager = gedit_window_get_ui_manager (window);

	data->action_group = gtk_action_group_new ("GeditChangecasePluginActions");
	gtk_action_group_set_translation_domain (data->action_group, 
						 GETTEXT_PACKAGE);
	gtk_action_group_add_actions (data->action_group,
				      action_entries,
				      G_N_ELEMENTS (action_entries), 
				      window);

	gtk_ui_manager_insert_action_group (manager, data->action_group, -1);

	data->ui_id = gtk_ui_manager_add_ui_from_string (manager,
							 submenu,
							 -1,
							 &error);
	if (data->ui_id == 0)
	{
		g_warning ("%s", error->message);
		free_window_data (data);
		return;
	}

	g_object_set_data_full (G_OBJECT (window), 
				WINDOW_DATA_KEY, 
				data,
				(GDestroyNotify) free_window_data);

	update_ui_real (window, data);
}

static void
impl_deactivate	(GeditPlugin *plugin,
		 GeditWindow *window)
{
	GtkUIManager *manager;
	WindowData *data;

	gedit_debug (DEBUG_PLUGINS);

	manager = gedit_window_get_ui_manager (window);

	data = (WindowData *) g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	gtk_ui_manager_remove_ui (manager, data->ui_id);
	gtk_ui_manager_remove_action_group (manager, data->action_group);

	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);	
}

static void
impl_update_ui (GeditPlugin *plugin,
		GeditWindow *window)
{
	WindowData *data;

	gedit_debug (DEBUG_PLUGINS);

	data = (WindowData *) g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	update_ui_real (window, data);
}

static void
gedit_changecase_plugin_class_init (GeditChangecasePluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GeditPluginClass *plugin_class = GEDIT_PLUGIN_CLASS (klass);

	object_class->finalize = gedit_changecase_plugin_finalize;

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
	plugin_class->update_ui = impl_update_ui;
}
