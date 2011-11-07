/*
 * gedit-session.c - Basic session management for gedit
 * This file is part of gedit
 *
 * Copyright (C) 2002 Ximian, Inc.
 * Copyright (C) 2005 - Paolo Maggi 
 *
 * Author: Federico Mena-Quintero <federico@ximian.com>
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
 * Modified by the gedit Team, 2002-2005. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <string.h>

#include <libxml/tree.h>
#include <libxml/xmlwriter.h>

#include "gedit-session.h"

#include "gedit-debug.h"
#include "gedit-plugins-engine.h"
#include "gedit-prefs-manager-app.h"
#include "gedit-metadata-manager.h"
#include "gedit-window.h"
#include "gedit-app.h"
#include "gedit-commands.h"
#include "dialogs/gedit-close-confirmation-dialog.h"
#include "smclient/eggsmclient.h"

/* The master client we use for SM */
static EggSMClient *master_client = NULL;

/* global var used during quit_requested */
static GSList *window_dirty_list;

static void	ask_next_confirmation	(void);

#define GEDIT_SESSION_LIST_OF_DOCS_TO_SAVE "gedit-session-list-of-docs-to-save-key"

static void
save_window_session (GKeyFile    *state_file,
		     const gchar *group_name,
		     GeditWindow      *window)
{
	const gchar *role;
	int width, height;
	GeditPanel *panel;
	GList *docs, *l;
	GPtrArray *doc_array;
	GeditDocument *active_document;
	gchar *uri;

	gedit_debug (DEBUG_SESSION);

	role = gtk_window_get_role (GTK_WINDOW (window));
	g_key_file_set_string (state_file, group_name, "role", role);
	gtk_window_get_size (GTK_WINDOW (window), &width, &height);
	g_key_file_set_integer (state_file, group_name, "width", width);
	g_key_file_set_integer (state_file, group_name, "height", height);

	panel = gedit_window_get_side_panel (window);
	g_key_file_set_boolean (state_file, group_name, "side-panel-visible",
				GTK_WIDGET_VISIBLE (panel));

	panel = gedit_window_get_bottom_panel (window);
	g_key_file_set_boolean (state_file, group_name, "bottom-panel-visible",
				GTK_WIDGET_VISIBLE (panel));

	active_document = gedit_window_get_active_document (window);
	if (active_document)
	{
	        uri = gedit_document_get_uri (active_document);
	        g_key_file_set_string (state_file, group_name,
				       "active-document", uri);
	}

	docs = gedit_window_get_documents (window);

	doc_array = g_ptr_array_new ();
	for (l = docs; l != NULL; l = g_list_next (l))
	{
		uri = gedit_document_get_uri (GEDIT_DOCUMENT (l->data));

		if (uri != NULL)
		        g_ptr_array_add (doc_array, uri);
			  
	}
	g_list_free (docs);	

	if (doc_array->len)
	{
	        guint i;
 
		g_key_file_set_string_list (state_file, group_name,
					    "documents",
					    (const char **)doc_array->pdata,
					    doc_array->len);
		for (i = 0; i < doc_array->len; i++)
		        g_free (doc_array->pdata[i]);
	}
	g_ptr_array_free (doc_array, TRUE);
}

static void
client_save_state_cb (EggSMClient *client,
		      GKeyFile    *state_file,
		      gpointer     user_data)
{
        const GList *windows;
	gchar *group_name;
	int n;

	windows = gedit_app_get_windows (gedit_app_get_default ());
	n = 1;

	while (windows != NULL)
	{
	        group_name = g_strdup_printf ("gedit window %d", n);
		save_window_session (state_file,
				     group_name,
				     GEDIT_WINDOW (windows->data));
		g_free (group_name);
		
		windows = g_list_next (windows);
		n++;
	}
}

static void
window_handled (GeditWindow *window)
{
	window_dirty_list = g_slist_remove (window_dirty_list, window);

	/* whee... we made it! */
	if (window_dirty_list == NULL)
	        egg_sm_client_will_quit (master_client, TRUE);
	else
		ask_next_confirmation ();
}

static void
window_state_change (GeditWindow *window,
		     GParamSpec  *pspec,
		     gpointer     data)
{
	GeditWindowState state;
	GList *unsaved_docs;
	GList *docs_to_save;
	GList *l;
	gboolean done = TRUE;

	state = gedit_window_get_state (window);

	/* we are still saving */
	if (state & GEDIT_WINDOW_STATE_SAVING)
		return;

	unsaved_docs = gedit_window_get_unsaved_documents (window);

	docs_to_save =	g_object_get_data (G_OBJECT (window),
					   GEDIT_SESSION_LIST_OF_DOCS_TO_SAVE);


	for (l = docs_to_save; l != NULL; l = l->next)
	{
		if (g_list_find (unsaved_docs, l->data))
		{
			done = FALSE;
			break;
		}
	}

	if (done)
	{
		g_signal_handlers_disconnect_by_func (window, window_state_change, data);
		g_list_free (docs_to_save);
		g_object_set_data (G_OBJECT (window),
				   GEDIT_SESSION_LIST_OF_DOCS_TO_SAVE,
				   NULL);

		window_handled (window);
	}

	g_list_free (unsaved_docs);
}

static void
close_confirmation_dialog_response_handler (GeditCloseConfirmationDialog *dlg,
					    gint                          response_id,
					    GeditWindow                  *window)
{
	GList *selected_documents;
	GSList *l;

	gedit_debug (DEBUG_COMMANDS);

	switch (response_id)
	{
		case GTK_RESPONSE_YES:
			/* save selected docs */

			g_signal_connect (window,
					  "notify::state",
					  G_CALLBACK (window_state_change),
					  NULL);

			selected_documents = gedit_close_confirmation_dialog_get_selected_documents (dlg);

			g_return_if_fail (g_object_get_data (G_OBJECT (window),
							     GEDIT_SESSION_LIST_OF_DOCS_TO_SAVE) == NULL);

			g_object_set_data (G_OBJECT (window),
					   GEDIT_SESSION_LIST_OF_DOCS_TO_SAVE,
					   selected_documents);

			_gedit_cmd_file_save_documents_list (window, selected_documents);

			/* FIXME: also need to lock the window to prevent further changes... */

			break;

		case GTK_RESPONSE_NO:
			/* dont save */
			window_handled (window);
			break;

		default:
			/* disconnect window_state_changed where needed */
			for (l = window_dirty_list; l != NULL; l = l->next)
				g_signal_handlers_disconnect_by_func (window,
						window_state_change, NULL);
			g_slist_free (window_dirty_list);
			window_dirty_list = NULL;

			/* cancel shutdown */
			egg_sm_client_will_quit (master_client, FALSE);

			break;
	}

	gtk_widget_destroy (GTK_WIDGET (dlg));
}

static void
show_confirmation_dialog (GeditWindow *window)
{
	GList *unsaved_docs;
	GtkWidget *dlg;

	gedit_debug (DEBUG_SESSION);

	unsaved_docs = gedit_window_get_unsaved_documents (window);

	g_return_if_fail (unsaved_docs != NULL);

	if (unsaved_docs->next == NULL)
	{
		/* There is only one unsaved document */
		GeditTab *tab;
		GeditDocument *doc;

		doc = GEDIT_DOCUMENT (unsaved_docs->data);

		tab = gedit_tab_get_from_document (doc);
		g_return_if_fail (tab != NULL);

		gedit_window_set_active_tab (window, tab);

		dlg = gedit_close_confirmation_dialog_new_single (
						GTK_WINDOW (window),
						doc,
						TRUE);
	}
	else
	{
		dlg = gedit_close_confirmation_dialog_new (GTK_WINDOW (window),
							   unsaved_docs,
							   TRUE);
	}

	g_list_free (unsaved_docs);

	g_signal_connect (dlg,
			  "response",
			  G_CALLBACK (close_confirmation_dialog_response_handler),
			  window);

	gtk_widget_show (dlg);
}

static void
ask_next_confirmation (void)
{
	g_return_if_fail (window_dirty_list != NULL);

	/* pop up the confirmation dialog for the first window
	 * in the dirty list. The next confirmation is asked once
	 * this one has been handled.
	 */
	show_confirmation_dialog (GEDIT_WINDOW (window_dirty_list->data));
}

/* quit_requested handler for the master client */
static void
client_quit_requested_cb (EggSMClient *client, gpointer data)
{
	GeditApp *app;
	const GList *l;

	gedit_debug (DEBUG_SESSION);

	app = gedit_app_get_default ();

	if (window_dirty_list != NULL)
	{
		g_critical ("global variable window_dirty_list not NULL");
		window_dirty_list = NULL;
	}

	for (l = gedit_app_get_windows (app); l != NULL; l = l->next)
	{
		if (gedit_window_get_unsaved_documents (GEDIT_WINDOW (l->data)) != NULL)
		{
			window_dirty_list = g_slist_prepend (window_dirty_list, l->data);
		}
	}

	/* no modified docs */
	if (window_dirty_list == NULL)
	{
		egg_sm_client_will_quit (client, TRUE);

		return;
	}

	ask_next_confirmation ();

	gedit_debug_message (DEBUG_SESSION, "END");
}

/* quit handler for the master client */
static void
client_quit_cb (EggSMClient *client, gpointer data)
{
#if 0
	gedit_debug (DEBUG_SESSION);

	if (!client->save_yourself_emitted)
		gedit_file_close_all ();

	gedit_debug_message (DEBUG_FILE, "All files closed.");
	
	matecomponent_mdi_destroy (MATECOMPONENT_MDI (gedit_mdi));
	
	gedit_debug_message (DEBUG_FILE, "Unref gedit_mdi.");

	g_object_unref (G_OBJECT (gedit_mdi));

	gedit_debug_message (DEBUG_FILE, "Unref gedit_mdi: DONE");

	gedit_debug_message (DEBUG_FILE, "Unref gedit_app_server.");

	matecomponent_object_unref (gedit_app_server);

	gedit_debug_message (DEBUG_FILE, "Unref gedit_app_server: DONE");
#endif

	gtk_main_quit ();
}

/**
 * gedit_session_init:
 * 
 * Initializes session management support.  This function should be called near
 * the beginning of the program.
 **/
void
gedit_session_init (void)
{
	gedit_debug (DEBUG_SESSION);
	
	if (master_client)
	  return;

	master_client = egg_sm_client_get ();
	g_signal_connect (master_client,
			  "save_state",
			  G_CALLBACK (client_save_state_cb),
			  NULL);
	g_signal_connect (master_client,
			  "quit_requested",
			  G_CALLBACK (client_quit_requested_cb),
			  NULL);
	g_signal_connect (master_client,
			  "quit",
			  G_CALLBACK (client_quit_cb),
			  NULL);		  
}

/**
 * gedit_session_is_restored:
 * 
 * Returns whether this gedit is running from a restarted session.
 * 
 * Return value: TRUE if the session manager restarted us, FALSE otherwise.
 * This should be used to determine whether to pay attention to command line
 * arguments in case the session was not restored.
 **/
gboolean
gedit_session_is_restored (void)
{
	gboolean restored;

	gedit_debug (DEBUG_SESSION);

	if (!master_client)
		return FALSE;

	restored = egg_sm_client_is_resumed (master_client);

	gedit_debug_message (DEBUG_SESSION, restored ? "RESTORED" : "NOT RESTORED");

	return restored;
}

static void
parse_window (GKeyFile *state_file, const char *group_name)
{
	GeditWindow *window;
	gchar *role, *active_document, **documents;
	int width, height;
	gboolean visible;
	GeditPanel *panel;
	GError *error = NULL;
  
	role = g_key_file_get_string (state_file, group_name, "role", NULL);

	gedit_debug_message (DEBUG_SESSION, "Window role: %s", role);

	window = _gedit_app_restore_window (gedit_app_get_default (), (gchar *) role);
	g_free (role);

	if (window == NULL)
	{
		g_warning ("Couldn't restore window");
		return;
	}

	width = g_key_file_get_integer (state_file, group_name,
					"width", &error);
	if (error)
	{
	        g_clear_error (&error);
		width = -1;
	}
	height = g_key_file_get_integer (state_file, group_name,
					 "height", &error);
	if (error)
	{
	        g_clear_error (&error);
		height = -1;
	}
	gtk_window_set_default_size (GTK_WINDOW (window), width, height);
  
 
	visible = g_key_file_get_boolean (state_file, group_name,
					  "side-panel-visible", &error);
	if (error)
	{
	        g_clear_error (&error);
		visible = FALSE;
	}
  
	panel = gedit_window_get_side_panel (window);
  
	if (visible)
	{
	        gedit_debug_message (DEBUG_SESSION, "Side panel visible");
		gtk_widget_show (GTK_WIDGET (panel));
	}
	else
	{
	      gedit_debug_message (DEBUG_SESSION, "Side panel _NOT_ visible");
	      gtk_widget_hide (GTK_WIDGET (panel));
	}
  
	visible = g_key_file_get_boolean (state_file, group_name,
					  "bottom-panel-visible", &error);
	if (error)
	{
	        g_clear_error (&error);
		visible = FALSE;
	}
  
	panel = gedit_window_get_bottom_panel (window);
	if (visible)
	{
	        gedit_debug_message (DEBUG_SESSION, "Bottom panel visible");
		gtk_widget_show (GTK_WIDGET (panel));
	}
	else
	{
	        gedit_debug_message (DEBUG_SESSION, "Bottom panel _NOT_ visible");
		gtk_widget_hide (GTK_WIDGET (panel));
	}

	active_document = g_key_file_get_string (state_file, group_name,
						 "active-document", NULL);
	documents = g_key_file_get_string_list (state_file, group_name,
						"documents", NULL, NULL);
	if (documents)
	{
	        int i;
		gboolean jump_to = FALSE;
  
		for (i = 0; documents[i]; i++)
		{
		        if (active_document != NULL)
			        jump_to = strcmp (active_document,
						  documents[i]) == 0;
  
			gedit_debug_message (DEBUG_SESSION,
					     "URI: %s (%s)",
					     documents[i],
					     jump_to ? "active" : "not active");
			gedit_window_create_tab_from_uri (window,
							  documents[i],
							  NULL,
							  0,
							  FALSE,
							  jump_to);
		}
		g_strfreev (documents);
	}
 
	g_free (active_document);
	
	gtk_widget_show (GTK_WIDGET (window));
}

/**
 * gedit_session_load:
 * 
 * Loads the session by fetching the necessary information from the session
 * manager and opening files.
 * 
 * Return value: TRUE if the session was loaded successfully, FALSE otherwise.
 **/
gboolean
gedit_session_load (void)
{
	GKeyFile *state_file;
	gchar **groups;
	int i;

	gedit_debug (DEBUG_SESSION);

	state_file = egg_sm_client_get_state_file (master_client);
	if (state_file == NULL)
	       return FALSE;

	groups = g_key_file_get_groups (state_file, NULL);

	for (i = 0; groups[i] != NULL; i++)
	{
		if (g_str_has_prefix (groups[i], "gedit window "))
		        parse_window (state_file, groups[i]);
	}

	g_strfreev (groups);
	g_key_file_free (state_file);

	return TRUE;
}
