/*
 * pluma-tab.c
 * This file is part of pluma
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA 02110-1301, USA.
 */
 
/*
 * Modified by the pluma Team, 2005. See the AUTHORS file for a 
 * list of people on the pluma Team.  
 * See the ChangeLog files for a list of changes. 
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>
#include <gio/gio.h>

#include "pluma-app.h"
#include "pluma-notebook.h"
#include "pluma-tab.h"
#include "pluma-utils.h"
#include "pluma-io-error-message-area.h"
#include "pluma-print-job.h"
#include "pluma-print-preview.h"
#include "pluma-progress-message-area.h"
#include "pluma-debug.h"
#include "pluma-prefs-manager-app.h"
#include "pluma-enum-types.h"

#if !GTK_CHECK_VERSION (2, 17, 1)
#include "pluma-message-area.h"
#endif

#define PLUMA_TAB_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), PLUMA_TYPE_TAB, PlumaTabPrivate))

#define PLUMA_TAB_KEY "PLUMA_TAB_KEY"

struct _PlumaTabPrivate
{
	PlumaTabState	        state;
	
	GtkWidget	       *view;
	GtkWidget	       *view_scrolled_window;

	GtkWidget	       *message_area;
	GtkWidget	       *print_preview;

	PlumaPrintJob          *print_job;

	/* tmp data for saving */
	gchar		       *tmp_save_uri;

	/* tmp data for loading */
	gint                    tmp_line_pos;
	const PlumaEncoding    *tmp_encoding;
	
	GTimer 		       *timer;
	guint		        times_called;

	PlumaDocumentSaveFlags	save_flags;

        gint                    auto_save_interval;
        guint                   auto_save_timeout;
        
	gint	                not_editable : 1;
	gint                    auto_save : 1;

	gint                    ask_if_externally_modified : 1;
};

G_DEFINE_TYPE(PlumaTab, pluma_tab, GTK_TYPE_VBOX)

enum
{
	PROP_0,
	PROP_NAME,
	PROP_STATE,
	PROP_AUTO_SAVE,
	PROP_AUTO_SAVE_INTERVAL
};

static gboolean pluma_tab_auto_save (PlumaTab *tab);

static void
install_auto_save_timeout (PlumaTab *tab)
{
	gint timeout;

	pluma_debug (DEBUG_TAB);

	g_return_if_fail (tab->priv->auto_save_timeout <= 0);
	g_return_if_fail (tab->priv->auto_save);
	g_return_if_fail (tab->priv->auto_save_interval > 0);
	
	g_return_if_fail (tab->priv->state != PLUMA_TAB_STATE_LOADING);
	g_return_if_fail (tab->priv->state != PLUMA_TAB_STATE_SAVING);
	g_return_if_fail (tab->priv->state != PLUMA_TAB_STATE_REVERTING);
	g_return_if_fail (tab->priv->state != PLUMA_TAB_STATE_LOADING_ERROR);
	g_return_if_fail (tab->priv->state != PLUMA_TAB_STATE_SAVING_ERROR);
	g_return_if_fail (tab->priv->state != PLUMA_TAB_STATE_SAVING_ERROR);
	g_return_if_fail (tab->priv->state != PLUMA_TAB_STATE_REVERTING_ERROR);

	/* Add a new timeout */
	timeout = g_timeout_add_seconds (tab->priv->auto_save_interval * 60,
					 (GSourceFunc) pluma_tab_auto_save,
					 tab);

	tab->priv->auto_save_timeout = timeout;
}

static gboolean
install_auto_save_timeout_if_needed (PlumaTab *tab)
{
	PlumaDocument *doc;

	pluma_debug (DEBUG_TAB);
	
	g_return_val_if_fail (tab->priv->auto_save_timeout <= 0, FALSE);
	g_return_val_if_fail ((tab->priv->state == PLUMA_TAB_STATE_NORMAL) ||
			      (tab->priv->state == PLUMA_TAB_STATE_SHOWING_PRINT_PREVIEW) ||
			      (tab->priv->state == PLUMA_TAB_STATE_CLOSING), FALSE);

	if (tab->priv->state == PLUMA_TAB_STATE_CLOSING)
		return FALSE;

	doc = pluma_tab_get_document (tab);

 	if (tab->priv->auto_save && 
 	    !pluma_document_is_untitled (doc) &&
 	    !pluma_document_get_readonly (doc))
 	{ 
 		install_auto_save_timeout (tab);
 		
 		return TRUE;
 	}
 	
 	return FALSE;
}

static void
remove_auto_save_timeout (PlumaTab *tab)
{
	pluma_debug (DEBUG_TAB);

	/* FIXME: check sugli stati */
	
	g_return_if_fail (tab->priv->auto_save_timeout > 0);
	
	g_source_remove (tab->priv->auto_save_timeout);
	tab->priv->auto_save_timeout = 0;
}

static void
pluma_tab_get_property (GObject    *object,
		        guint       prop_id,
		        GValue     *value,
		        GParamSpec *pspec)
{
	PlumaTab *tab = PLUMA_TAB (object);

	switch (prop_id)
	{
		case PROP_NAME:
			g_value_take_string (value,
					     _pluma_tab_get_name (tab));
			break;
		case PROP_STATE:
			g_value_set_enum (value,
					  pluma_tab_get_state (tab));
			break;			
		case PROP_AUTO_SAVE:
			g_value_set_boolean (value,
					     pluma_tab_get_auto_save_enabled (tab));
			break;
		case PROP_AUTO_SAVE_INTERVAL:
			g_value_set_int (value,
					 pluma_tab_get_auto_save_interval (tab));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;			
	}
}

static void
pluma_tab_set_property (GObject      *object,
		        guint         prop_id,
		        const GValue *value,
		        GParamSpec   *pspec)
{
	PlumaTab *tab = PLUMA_TAB (object);

	switch (prop_id)
	{
		case PROP_AUTO_SAVE:
			pluma_tab_set_auto_save_enabled (tab,
							 g_value_get_boolean (value));
			break;
		case PROP_AUTO_SAVE_INTERVAL:
			pluma_tab_set_auto_save_interval (tab,
							  g_value_get_int (value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;			
	}
}

static void
pluma_tab_finalize (GObject *object)
{
	PlumaTab *tab = PLUMA_TAB (object);

	if (tab->priv->timer != NULL)
		g_timer_destroy (tab->priv->timer);

	g_free (tab->priv->tmp_save_uri);

	if (tab->priv->auto_save_timeout > 0)
		remove_auto_save_timeout (tab);

	G_OBJECT_CLASS (pluma_tab_parent_class)->finalize (object);
}

static void 
pluma_tab_class_init (PlumaTabClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = pluma_tab_finalize;
	object_class->get_property = pluma_tab_get_property;
	object_class->set_property = pluma_tab_set_property;
	
	g_object_class_install_property (object_class,
					 PROP_NAME,
					 g_param_spec_string ("name",
							      "Name",
							      "The tab's name",
							      NULL,
							      G_PARAM_READABLE |
							      G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (object_class,
					 PROP_STATE,
					 g_param_spec_enum ("state",
							    "State",
							    "The tab's state",
							    PLUMA_TYPE_TAB_STATE,
							    PLUMA_TAB_STATE_NORMAL,
							    G_PARAM_READABLE |
							    G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (object_class,
					 PROP_AUTO_SAVE,
					 g_param_spec_boolean ("autosave",
							       "Autosave",
							       "Autosave feature",
							       TRUE,
							       G_PARAM_READWRITE |
							       G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (object_class,
					 PROP_AUTO_SAVE_INTERVAL,
					 g_param_spec_int ("autosave-interval",
							   "AutosaveInterval",
							   "Time between two autosaves",
							   0,
							   G_MAXINT,
							   0,
							   G_PARAM_READWRITE |
							   G_PARAM_STATIC_STRINGS));

	g_type_class_add_private (object_class, sizeof (PlumaTabPrivate));
}

/**
 * pluma_tab_get_state:
 * @tab: a #PlumaTab
 *
 * Gets the #PlumaTabState of @tab.
 *
 * Returns: the #PlumaTabState of @tab
 */
PlumaTabState
pluma_tab_get_state (PlumaTab *tab)
{
	g_return_val_if_fail (PLUMA_IS_TAB (tab), PLUMA_TAB_STATE_NORMAL);
	
	return tab->priv->state;
}

static void
set_cursor_according_to_state (GtkTextView   *view,
			       PlumaTabState  state)
{
	GdkCursor *cursor;
	GdkWindow *text_window;
	GdkWindow *left_window;

	text_window = gtk_text_view_get_window (view, GTK_TEXT_WINDOW_TEXT);
	left_window = gtk_text_view_get_window (view, GTK_TEXT_WINDOW_LEFT);

	if ((state == PLUMA_TAB_STATE_LOADING)          ||
	    (state == PLUMA_TAB_STATE_REVERTING)        ||
	    (state == PLUMA_TAB_STATE_SAVING)           ||
	    (state == PLUMA_TAB_STATE_PRINTING)         ||
	    (state == PLUMA_TAB_STATE_PRINT_PREVIEWING) ||
	    (state == PLUMA_TAB_STATE_CLOSING))
	{
		cursor = gdk_cursor_new_for_display (
				gtk_widget_get_display (GTK_WIDGET (view)),
				GDK_WATCH);

		if (text_window != NULL)
			gdk_window_set_cursor (text_window, cursor);
		if (left_window != NULL)
			gdk_window_set_cursor (left_window, cursor);

		gdk_cursor_unref (cursor);
	}
	else
	{
		cursor = gdk_cursor_new_for_display (
				gtk_widget_get_display (GTK_WIDGET (view)),
				GDK_XTERM);

		if (text_window != NULL)
			gdk_window_set_cursor (text_window, cursor);
		if (left_window != NULL)
			gdk_window_set_cursor (left_window, NULL);

		gdk_cursor_unref (cursor);
	}
}

static void
view_realized (GtkTextView *view,
	       PlumaTab    *tab)
{
	set_cursor_according_to_state (view, tab->priv->state);
}

static void
set_view_properties_according_to_state (PlumaTab      *tab,
					PlumaTabState  state)
{
	gboolean val;

	val = ((state == PLUMA_TAB_STATE_NORMAL) &&
	       (tab->priv->print_preview == NULL) &&
	       !tab->priv->not_editable);
	gtk_text_view_set_editable (GTK_TEXT_VIEW (tab->priv->view), val);

	val = ((state != PLUMA_TAB_STATE_LOADING) &&
	       (state != PLUMA_TAB_STATE_CLOSING));
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (tab->priv->view), val);

	val = ((state != PLUMA_TAB_STATE_LOADING) &&
	       (state != PLUMA_TAB_STATE_CLOSING) &&
	       (pluma_prefs_manager_get_highlight_current_line ()));
	gtk_source_view_set_highlight_current_line (GTK_SOURCE_VIEW (tab->priv->view), val);
}

static void
pluma_tab_set_state (PlumaTab      *tab,
		     PlumaTabState  state)
{
	g_return_if_fail (PLUMA_IS_TAB (tab));
	g_return_if_fail ((state >= 0) && (state < PLUMA_TAB_NUM_OF_STATES));

	if (tab->priv->state == state)
		return;

	tab->priv->state = state;

	set_view_properties_according_to_state (tab, state);

	if ((state == PLUMA_TAB_STATE_LOADING_ERROR) || /* FIXME: add other states if needed */
	    (state == PLUMA_TAB_STATE_SHOWING_PRINT_PREVIEW))
	{
		gtk_widget_hide (tab->priv->view_scrolled_window);
	}
	else
	{
		if (tab->priv->print_preview == NULL)
			gtk_widget_show (tab->priv->view_scrolled_window);
	}

	set_cursor_according_to_state (GTK_TEXT_VIEW (tab->priv->view),
				       state);

	g_object_notify (G_OBJECT (tab), "state");		
}

static void 
document_uri_notify_handler (PlumaDocument *document,
			     GParamSpec    *pspec,
			     PlumaTab      *tab)
{
	pluma_debug (DEBUG_TAB);
	
	/* Notify the change in the URI */
	g_object_notify (G_OBJECT (tab), "name");
}

static void 
document_shortname_notify_handler (PlumaDocument *document,
				   GParamSpec    *pspec,
				   PlumaTab      *tab)
{
	pluma_debug (DEBUG_TAB);
	
	/* Notify the change in the shortname */
	g_object_notify (G_OBJECT (tab), "name");
}

static void
document_modified_changed (GtkTextBuffer *document,
			   PlumaTab      *tab)
{
	g_object_notify (G_OBJECT (tab), "name");
}

static void
set_message_area (PlumaTab  *tab,
		  GtkWidget *message_area)
{
	if (tab->priv->message_area == message_area)
		return;

	if (tab->priv->message_area != NULL)
		gtk_widget_destroy (tab->priv->message_area);

	tab->priv->message_area = message_area;

	if (message_area == NULL)
		return;

	gtk_box_pack_start (GTK_BOX (tab),
			    tab->priv->message_area,
			    FALSE,
			    FALSE,
			    0);		

	g_object_add_weak_pointer (G_OBJECT (tab->priv->message_area), 
				   (gpointer *)&tab->priv->message_area);
}

static void
remove_tab (PlumaTab *tab)
{
	PlumaNotebook *notebook;

	notebook = PLUMA_NOTEBOOK (gtk_widget_get_parent (GTK_WIDGET (tab)));

	pluma_notebook_remove_tab (notebook, tab);
}

static void 
io_loading_error_message_area_response (GtkWidget        *message_area,
					gint              response_id,
					PlumaTab         *tab)
{
	PlumaDocument *doc;
	PlumaView *view;
	gchar *uri;
	const PlumaEncoding *encoding;

	doc = pluma_tab_get_document (tab);
	g_return_if_fail (PLUMA_IS_DOCUMENT (doc));

	view = pluma_tab_get_view (tab);
	g_return_if_fail (PLUMA_IS_VIEW (view));

	uri = pluma_document_get_uri (doc);
	g_return_if_fail (uri != NULL);

	switch (response_id)
	{
		case GTK_RESPONSE_OK:
			encoding = pluma_conversion_error_message_area_get_encoding (
					GTK_WIDGET (message_area));

			if (encoding != NULL)
			{
				tab->priv->tmp_encoding = encoding;
			}

			set_message_area (tab, NULL);
			pluma_tab_set_state (tab, PLUMA_TAB_STATE_LOADING);

			g_return_if_fail (tab->priv->auto_save_timeout <= 0);

			pluma_document_load (doc,
					     uri,
					     tab->priv->tmp_encoding,
					     tab->priv->tmp_line_pos,
					     FALSE);
			break;
		case GTK_RESPONSE_YES:
			/* This means that we want to edit the document anyway */
			set_message_area (tab, NULL);
			_pluma_document_set_readonly (doc, FALSE);
			break;
		case GTK_RESPONSE_NO:
			/* We don't want to edit the document just show it */
			set_message_area (tab, NULL);
			break;
		default:
			_pluma_recent_remove (PLUMA_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (tab))), uri);

			remove_tab (tab);
			break;
	}

	g_free (uri);
}

static void 
file_already_open_warning_message_area_response (GtkWidget   *message_area,
						 gint         response_id,
						 PlumaTab    *tab)
{
	PlumaView *view;
	
	view = pluma_tab_get_view (tab);
	
	if (response_id == GTK_RESPONSE_YES)
	{
		tab->priv->not_editable = FALSE;
		
		gtk_text_view_set_editable (GTK_TEXT_VIEW (view),
					    TRUE);
	}

	gtk_widget_destroy (message_area);

	gtk_widget_grab_focus (GTK_WIDGET (view));	
}

static void
load_cancelled (GtkWidget        *area,
                gint              response_id,
                PlumaTab         *tab)
{
	g_return_if_fail (PLUMA_IS_PROGRESS_MESSAGE_AREA (tab->priv->message_area));
	
	g_object_ref (tab);
	pluma_document_load_cancel (pluma_tab_get_document (tab));
	g_object_unref (tab);	
}

static void 
unrecoverable_reverting_error_message_area_response (GtkWidget        *message_area,
						     gint              response_id,
						     PlumaTab         *tab)
{
	PlumaView *view;
	
	pluma_tab_set_state (tab,
			     PLUMA_TAB_STATE_NORMAL);

	set_message_area (tab, NULL);

	view = pluma_tab_get_view (tab);

	gtk_widget_grab_focus (GTK_WIDGET (view));
	
	install_auto_save_timeout_if_needed (tab);	
}

#define MAX_MSG_LENGTH 100

static void
show_loading_message_area (PlumaTab *tab)
{
	GtkWidget *area;
	PlumaDocument *doc = NULL;
	gchar *name;
	gchar *dirname = NULL;
	gchar *msg = NULL;
	gchar *name_markup;
	gchar *dirname_markup;
	gint len;

	if (tab->priv->message_area != NULL)
		return;

	pluma_debug (DEBUG_TAB);
		
	doc = pluma_tab_get_document (tab);
	g_return_if_fail (doc != NULL);

	name = pluma_document_get_short_name_for_display (doc);
	len = g_utf8_strlen (name, -1);

	/* if the name is awfully long, truncate it and be done with it,
	 * otherwise also show the directory (ellipsized if needed)
	 */
	if (len > MAX_MSG_LENGTH)
	{
		gchar *str;

		str = pluma_utils_str_middle_truncate (name, MAX_MSG_LENGTH);
		g_free (name);
		name = str;
	}
	else
	{
		GFile *file;

		file = pluma_document_get_location (doc);
		if (file != NULL)
		{
			gchar *str;

			str = pluma_utils_location_get_dirname_for_display (file);
			g_object_unref (file);

			/* use the remaining space for the dir, but use a min of 20 chars
			 * so that we do not end up with a dirname like "(a...b)".
			 * This means that in the worst case when the filename is long 99
			 * we have a title long 99 + 20, but I think it's a rare enough
			 * case to be acceptable. It's justa darn title afterall :)
			 */
			dirname = pluma_utils_str_middle_truncate (str, 
								   MAX (20, MAX_MSG_LENGTH - len));
			g_free (str);
		}
	}

	name_markup = g_markup_printf_escaped ("<b>%s</b>", name);

	if (tab->priv->state == PLUMA_TAB_STATE_REVERTING)
	{
		if (dirname != NULL)
		{
			dirname_markup = g_markup_printf_escaped ("<b>%s</b>", dirname);

			/* Translators: the first %s is a file name (e.g. test.txt) the second one
			   is a directory (e.g. ssh://master.gnome.org/home/users/paolo) */
			msg = g_strdup_printf (_("Reverting %s from %s"),
					       name_markup,
					       dirname_markup);
			g_free (dirname_markup);
		}
		else
		{
			msg = g_strdup_printf (_("Reverting %s"), 
					       name_markup);
		}
		
		area = pluma_progress_message_area_new (GTK_STOCK_REVERT_TO_SAVED,
							msg,
							TRUE);
	}
	else
	{
		if (dirname != NULL)
		{
			dirname_markup = g_markup_printf_escaped ("<b>%s</b>", dirname);

			/* Translators: the first %s is a file name (e.g. test.txt) the second one
			   is a directory (e.g. ssh://master.gnome.org/home/users/paolo) */
			msg = g_strdup_printf (_("Loading %s from %s"),
					       name_markup,
					       dirname_markup);
			g_free (dirname_markup);
		}
		else
		{
			msg = g_strdup_printf (_("Loading %s"), 
					       name_markup);
		}

		area = pluma_progress_message_area_new (GTK_STOCK_OPEN,
							msg,
							TRUE);
	}

	g_signal_connect (area,
			  "response",
			  G_CALLBACK (load_cancelled),
			  tab);
						 
	gtk_widget_show (area);

	set_message_area (tab, area);

	g_free (msg);
	g_free (name);
	g_free (name_markup);
	g_free (dirname);
}

static void
show_saving_message_area (PlumaTab *tab)
{
	GtkWidget *area;
	PlumaDocument *doc = NULL;
	gchar *short_name;
	gchar *from;
	gchar *to = NULL;
	gchar *from_markup;
	gchar *to_markup;
	gchar *msg = NULL;
	gint len;

	g_return_if_fail (tab->priv->tmp_save_uri != NULL);
	
	if (tab->priv->message_area != NULL)
		return;
	
	pluma_debug (DEBUG_TAB);
		
	doc = pluma_tab_get_document (tab);
	g_return_if_fail (doc != NULL);

	short_name = pluma_document_get_short_name_for_display (doc);

	len = g_utf8_strlen (short_name, -1);

	/* if the name is awfully long, truncate it and be done with it,
	 * otherwise also show the directory (ellipsized if needed)
	 */
	if (len > MAX_MSG_LENGTH)
	{
		from = pluma_utils_str_middle_truncate (short_name, 
							MAX_MSG_LENGTH);
		g_free (short_name);
	}
	else
	{
		gchar *str;

		from = short_name;

		to = pluma_utils_uri_for_display (tab->priv->tmp_save_uri);

		str = pluma_utils_str_middle_truncate (to, 
						       MAX (20, MAX_MSG_LENGTH - len));
		g_free (to);
			
		to = str;
	}

	from_markup = g_markup_printf_escaped ("<b>%s</b>", from);

	if (to != NULL)
	{
		to_markup = g_markup_printf_escaped ("<b>%s</b>", to);

		/* Translators: the first %s is a file name (e.g. test.txt) the second one
		   is a directory (e.g. ssh://master.gnome.org/home/users/paolo) */
		msg = g_strdup_printf (_("Saving %s to %s"),
				       from_markup,
				       to_markup);
		g_free (to_markup);
	}
	else
	{
		msg = g_strdup_printf (_("Saving %s"), from_markup);
	}

	area = pluma_progress_message_area_new (GTK_STOCK_SAVE,
						msg,
						FALSE);

	gtk_widget_show (area);

	set_message_area (tab, area);

	g_free (msg);
	g_free (to);
	g_free (from);
	g_free (from_markup);
}

static void
message_area_set_progress (PlumaTab *tab,
			   goffset   size,
			   goffset   total_size)
{
	if (tab->priv->message_area == NULL)
		return;

	pluma_debug_message (DEBUG_TAB, "%" G_GUINT64_FORMAT "/%" G_GUINT64_FORMAT, size, total_size);

	g_return_if_fail (PLUMA_IS_PROGRESS_MESSAGE_AREA (tab->priv->message_area));
	
	if (total_size == 0)
	{
		if (size != 0)
			pluma_progress_message_area_pulse (
					PLUMA_PROGRESS_MESSAGE_AREA (tab->priv->message_area));	
		else
			pluma_progress_message_area_set_fraction (
				PLUMA_PROGRESS_MESSAGE_AREA (tab->priv->message_area),
				0);
	}
	else
	{
		gdouble frac;

		frac = (gdouble)size / (gdouble)total_size;

		pluma_progress_message_area_set_fraction (
				PLUMA_PROGRESS_MESSAGE_AREA (tab->priv->message_area),
				frac);		
	}
}

static void
document_loading (PlumaDocument *document,
		  goffset        size,
		  goffset        total_size,
		  PlumaTab      *tab)
{
	gdouble et;
	gdouble total_time;

	g_return_if_fail ((tab->priv->state == PLUMA_TAB_STATE_LOADING) ||
		 	  (tab->priv->state == PLUMA_TAB_STATE_REVERTING));

	pluma_debug_message (DEBUG_TAB, "%" G_GUINT64_FORMAT "/%" G_GUINT64_FORMAT, size, total_size);

	if (tab->priv->timer == NULL)
	{
		g_return_if_fail (tab->priv->times_called == 0);
		tab->priv->timer = g_timer_new ();
	}

	et = g_timer_elapsed (tab->priv->timer, NULL);

	/* et : total_time = size : total_size */
	total_time = (et * total_size) / size;

	if ((total_time - et) > 3.0)
	{
		show_loading_message_area (tab);
	}

	message_area_set_progress (tab, size, total_size);
}

static gboolean
remove_tab_idle (PlumaTab *tab)
{
	remove_tab (tab);

	return FALSE;
}

static void
document_loaded (PlumaDocument *document,
		 const GError  *error,
		 PlumaTab      *tab)
{
	GtkWidget *emsg;
	GFile *location;
	gchar *uri;
	const PlumaEncoding *encoding;

	g_return_if_fail ((tab->priv->state == PLUMA_TAB_STATE_LOADING) ||
			  (tab->priv->state == PLUMA_TAB_STATE_REVERTING));
	g_return_if_fail (tab->priv->auto_save_timeout <= 0);

	if (tab->priv->timer != NULL)
	{
		g_timer_destroy (tab->priv->timer);
		tab->priv->timer = NULL;
	}
	tab->priv->times_called = 0;

	set_message_area (tab, NULL);

	location = pluma_document_get_location (document);
	uri = pluma_document_get_uri (document);

	/* if the error is CONVERSION FALLBACK don't treat it as a normal error */
	if (error != NULL &&
	    (error->domain != PLUMA_DOCUMENT_ERROR || error->code != PLUMA_DOCUMENT_ERROR_CONVERSION_FALLBACK))
	{
		if (tab->priv->state == PLUMA_TAB_STATE_LOADING)
			pluma_tab_set_state (tab, PLUMA_TAB_STATE_LOADING_ERROR);
		else
			pluma_tab_set_state (tab, PLUMA_TAB_STATE_REVERTING_ERROR);

		encoding = pluma_document_get_encoding (document);

		if (error->domain == G_IO_ERROR &&
		    error->code == G_IO_ERROR_CANCELLED)
		{
			/* remove the tab, but in an idle handler, since
			 * we are in the handler of doc loaded and we 
			 * don't want doc and tab to be finalized now.
			 */
			g_idle_add ((GSourceFunc) remove_tab_idle, tab);

			goto end;
		}
		else
		{
			_pluma_recent_remove (PLUMA_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (tab))), uri);

			if (tab->priv->state == PLUMA_TAB_STATE_LOADING_ERROR)
			{
				emsg = pluma_io_loading_error_message_area_new (uri,
										tab->priv->tmp_encoding,
										error);
				g_signal_connect (emsg,
						  "response",
						  G_CALLBACK (io_loading_error_message_area_response),
						  tab);
			}
			else
			{
				g_return_if_fail (tab->priv->state == PLUMA_TAB_STATE_REVERTING_ERROR);
				
				emsg = pluma_unrecoverable_reverting_error_message_area_new (uri,
											     error);

				g_signal_connect (emsg,
						  "response",
						  G_CALLBACK (unrecoverable_reverting_error_message_area_response),
						  tab);
			}

			set_message_area (tab, emsg);
		}

#if !GTK_CHECK_VERSION (2, 17, 1)
		pluma_message_area_set_default_response (PLUMA_MESSAGE_AREA (emsg),
							 GTK_RESPONSE_CANCEL);
#else
		gtk_info_bar_set_default_response (GTK_INFO_BAR (emsg),
						   GTK_RESPONSE_CANCEL);
#endif

		gtk_widget_show (emsg);

		g_object_unref (location);
		g_free (uri);

		return;
	}
	else
	{
		gchar *mime;
		GList *all_documents;
		GList *l;

		g_return_if_fail (uri != NULL);

		mime = pluma_document_get_mime_type (document);
		_pluma_recent_add (PLUMA_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (tab))),
				   uri,
				   mime);
		g_free (mime);

		if (error &&
		    error->domain == PLUMA_DOCUMENT_ERROR &&
		    error->code == PLUMA_DOCUMENT_ERROR_CONVERSION_FALLBACK)
		{
			GtkWidget *emsg;

			_pluma_document_set_readonly (document, TRUE);

			emsg = pluma_io_loading_error_message_area_new (uri,
									tab->priv->tmp_encoding,
									error);

			set_message_area (tab, emsg);

			g_signal_connect (emsg,
					  "response",
					  G_CALLBACK (io_loading_error_message_area_response),
					  tab);

#if !GTK_CHECK_VERSION (2, 17, 1)
			pluma_message_area_set_default_response (PLUMA_MESSAGE_AREA (emsg),
								 GTK_RESPONSE_CANCEL);
#else
			gtk_info_bar_set_default_response (GTK_INFO_BAR (emsg),
							   GTK_RESPONSE_CANCEL);
#endif

			gtk_widget_show (emsg);
		}

		/* Scroll to the cursor when the document is loaded */
		pluma_view_scroll_to_cursor (PLUMA_VIEW (tab->priv->view));

		all_documents = pluma_app_get_documents (pluma_app_get_default ());

		for (l = all_documents; l != NULL; l = g_list_next (l))
		{
			PlumaDocument *d = PLUMA_DOCUMENT (l->data);
			
			if (d != document)
			{
				GFile *loc;

				loc = pluma_document_get_location (d);

				if ((loc != NULL) &&
			    	    g_file_equal (location, loc))
			    	{
			    		GtkWidget *w;
			    		PlumaView *view;

			    		view = pluma_tab_get_view (tab);

			    		tab->priv->not_editable = TRUE;

			    		w = pluma_file_already_open_warning_message_area_new (uri);

					set_message_area (tab, w);

#if !GTK_CHECK_VERSION (2, 17, 1)
					pluma_message_area_set_default_response (PLUMA_MESSAGE_AREA (w),
										 GTK_RESPONSE_CANCEL);
#else
					gtk_info_bar_set_default_response (GTK_INFO_BAR (w),
									   GTK_RESPONSE_CANCEL);
#endif

					gtk_widget_show (w);

					g_signal_connect (w,
							  "response",
							  G_CALLBACK (file_already_open_warning_message_area_response),
							  tab);

			    		g_object_unref (loc);
			    		break;
			    	}
			    	
			    	if (loc != NULL)
					g_object_unref (loc);
			}
		}

		g_list_free (all_documents);

		pluma_tab_set_state (tab, PLUMA_TAB_STATE_NORMAL);
		
		install_auto_save_timeout_if_needed (tab);

		tab->priv->ask_if_externally_modified = TRUE;
	}

 end:
	g_object_unref (location);
	g_free (uri);

	tab->priv->tmp_line_pos = 0;
	tab->priv->tmp_encoding = NULL;
}

static void
document_saving (PlumaDocument    *document,
		 goffset  size,
		 goffset  total_size,
		 PlumaTab         *tab)
{
	gdouble et;
	gdouble total_time;

	g_return_if_fail (tab->priv->state == PLUMA_TAB_STATE_SAVING);

	pluma_debug_message (DEBUG_TAB, "%" G_GUINT64_FORMAT "/%" G_GUINT64_FORMAT, size, total_size);


	if (tab->priv->timer == NULL)
	{
		g_return_if_fail (tab->priv->times_called == 0);
		tab->priv->timer = g_timer_new ();
	}

	et = g_timer_elapsed (tab->priv->timer, NULL);

	/* et : total_time = size : total_size */
	total_time = (et * total_size)/size;

	if ((total_time - et) > 3.0)
	{
		show_saving_message_area (tab);
	}

	message_area_set_progress (tab, size, total_size);

	tab->priv->times_called++;
}

static void
end_saving (PlumaTab *tab)
{
	/* Reset tmp data for saving */
	g_free (tab->priv->tmp_save_uri);
	tab->priv->tmp_save_uri = NULL;
	tab->priv->tmp_encoding = NULL;
	
	install_auto_save_timeout_if_needed (tab);
}

static void 
unrecoverable_saving_error_message_area_response (GtkWidget        *message_area,
						  gint              response_id,
						  PlumaTab         *tab)
{
	PlumaView *view;
	
	if (tab->priv->print_preview != NULL)
		pluma_tab_set_state (tab, PLUMA_TAB_STATE_SHOWING_PRINT_PREVIEW);
	else
		pluma_tab_set_state (tab, PLUMA_TAB_STATE_NORMAL);

	end_saving (tab);
	
	set_message_area (tab, NULL);

	view = pluma_tab_get_view (tab);

	gtk_widget_grab_focus (GTK_WIDGET (view));	
}

static void 
no_backup_error_message_area_response (GtkWidget        *message_area,
				       gint              response_id,
				       PlumaTab         *tab)
{
	if (response_id == GTK_RESPONSE_YES)
	{
		PlumaDocument *doc;

		doc = pluma_tab_get_document (tab);
		g_return_if_fail (PLUMA_IS_DOCUMENT (doc));

		set_message_area (tab, NULL);

		g_return_if_fail (tab->priv->tmp_save_uri != NULL);
		g_return_if_fail (tab->priv->tmp_encoding != NULL);

		pluma_tab_set_state (tab, PLUMA_TAB_STATE_SAVING);

		/* don't bug the user again with this... */
		tab->priv->save_flags |= PLUMA_DOCUMENT_SAVE_IGNORE_BACKUP;

		g_return_if_fail (tab->priv->auto_save_timeout <= 0);
		
		/* Force saving */
		pluma_document_save (doc, tab->priv->save_flags);
	}
	else
	{
		unrecoverable_saving_error_message_area_response (message_area,
								  response_id,
								  tab);
	}
}

static void
externally_modified_error_message_area_response (GtkWidget        *message_area,
						 gint              response_id,
						 PlumaTab         *tab)
{
	if (response_id == GTK_RESPONSE_YES)
	{
		PlumaDocument *doc;

		doc = pluma_tab_get_document (tab);
		g_return_if_fail (PLUMA_IS_DOCUMENT (doc));
		
		set_message_area (tab, NULL);

		g_return_if_fail (tab->priv->tmp_save_uri != NULL);
		g_return_if_fail (tab->priv->tmp_encoding != NULL);

		pluma_tab_set_state (tab, PLUMA_TAB_STATE_SAVING);

		g_return_if_fail (tab->priv->auto_save_timeout <= 0);
		
		/* ignore mtime should not be persisted in save flags across saves */

		/* Force saving */
		pluma_document_save (doc, tab->priv->save_flags | PLUMA_DOCUMENT_SAVE_IGNORE_MTIME);
	}
	else
	{		
		unrecoverable_saving_error_message_area_response (message_area,
								  response_id,
								  tab);
	}
}

static void 
recoverable_saving_error_message_area_response (GtkWidget        *message_area,
						gint              response_id,
						PlumaTab         *tab)
{
	PlumaDocument *doc;

	doc = pluma_tab_get_document (tab);
	g_return_if_fail (PLUMA_IS_DOCUMENT (doc));
	
	if (response_id == GTK_RESPONSE_OK)
	{
		const PlumaEncoding *encoding;
		
		encoding = pluma_conversion_error_message_area_get_encoding (
									GTK_WIDGET (message_area));

		g_return_if_fail (encoding != NULL);

		set_message_area (tab, NULL);

		g_return_if_fail (tab->priv->tmp_save_uri != NULL);
				
		pluma_tab_set_state (tab, PLUMA_TAB_STATE_SAVING);
			
		tab->priv->tmp_encoding = encoding;

		pluma_debug_message (DEBUG_TAB, "Force saving with URI '%s'", tab->priv->tmp_save_uri);
			 
		g_return_if_fail (tab->priv->auto_save_timeout <= 0);
		
		pluma_document_save_as (doc,
					tab->priv->tmp_save_uri,
					tab->priv->tmp_encoding,
					tab->priv->save_flags);
	}
	else
	{		
		unrecoverable_saving_error_message_area_response (message_area,
								  response_id,
								  tab);
	}
}

static void
document_saved (PlumaDocument *document,
		const GError  *error,
		PlumaTab      *tab)
{
	GtkWidget *emsg;

	g_return_if_fail (tab->priv->state == PLUMA_TAB_STATE_SAVING);

	g_return_if_fail (tab->priv->tmp_save_uri != NULL);
	g_return_if_fail (tab->priv->tmp_encoding != NULL);	
	g_return_if_fail (tab->priv->auto_save_timeout <= 0);
	
	g_timer_destroy (tab->priv->timer);
	tab->priv->timer = NULL;
	tab->priv->times_called = 0;
	
	set_message_area (tab, NULL);
	
	if (error != NULL)
	{
		pluma_tab_set_state (tab, PLUMA_TAB_STATE_SAVING_ERROR);
		
		if (error->domain == PLUMA_DOCUMENT_ERROR &&
		    error->code == PLUMA_DOCUMENT_ERROR_EXTERNALLY_MODIFIED)
		{
			/* This error is recoverable */
			emsg = pluma_externally_modified_saving_error_message_area_new (
							tab->priv->tmp_save_uri, 
							error);
			g_return_if_fail (emsg != NULL);

			set_message_area (tab, emsg);

			g_signal_connect (emsg,
					  "response",
					  G_CALLBACK (externally_modified_error_message_area_response),
					  tab);
		}
		else if ((error->domain == PLUMA_DOCUMENT_ERROR &&
			  error->code == PLUMA_DOCUMENT_ERROR_CANT_CREATE_BACKUP) ||
		         (error->domain == G_IO_ERROR &&
			  error->code == G_IO_ERROR_CANT_CREATE_BACKUP))
		{
			/* This error is recoverable */
			emsg = pluma_no_backup_saving_error_message_area_new (
							tab->priv->tmp_save_uri, 
							error);
			g_return_if_fail (emsg != NULL);

			set_message_area (tab, emsg);

			g_signal_connect (emsg,
					  "response",
					  G_CALLBACK (no_backup_error_message_area_response),
					  tab);
		}
		else if (error->domain == PLUMA_DOCUMENT_ERROR ||
			 (error->domain == G_IO_ERROR &&
			  error->code != G_IO_ERROR_INVALID_DATA &&
			  error->code != G_IO_ERROR_PARTIAL_INPUT))
		{
			/* These errors are _NOT_ recoverable */
			_pluma_recent_remove  (PLUMA_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (tab))),
					       tab->priv->tmp_save_uri);

			emsg = pluma_unrecoverable_saving_error_message_area_new (tab->priv->tmp_save_uri, 
								  error);
			g_return_if_fail (emsg != NULL);
	
			set_message_area (tab, emsg);

			g_signal_connect (emsg,
					  "response",
					  G_CALLBACK (unrecoverable_saving_error_message_area_response),
					  tab);
		}
		else
		{
			/* This error is recoverable */
			g_return_if_fail (error->domain == G_CONVERT_ERROR ||
			                  error->domain == G_IO_ERROR);

			emsg = pluma_conversion_error_while_saving_message_area_new (
									tab->priv->tmp_save_uri,
									tab->priv->tmp_encoding,
									error);

			set_message_area (tab, emsg);

			g_signal_connect (emsg,
					  "response",
					  G_CALLBACK (recoverable_saving_error_message_area_response),
					  tab);
		}

#if !GTK_CHECK_VERSION (2, 17, 1)
		pluma_message_area_set_default_response (PLUMA_MESSAGE_AREA (emsg),
							 GTK_RESPONSE_CANCEL);
#else
		gtk_info_bar_set_default_response (GTK_INFO_BAR (emsg),
						   GTK_RESPONSE_CANCEL);
#endif

		gtk_widget_show (emsg);
	}
	else
	{
		gchar *mime = pluma_document_get_mime_type (document);

		_pluma_recent_add (PLUMA_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (tab))),
				   tab->priv->tmp_save_uri,
				   mime);
		g_free (mime);

		if (tab->priv->print_preview != NULL)
			pluma_tab_set_state (tab, PLUMA_TAB_STATE_SHOWING_PRINT_PREVIEW);
		else
			pluma_tab_set_state (tab, PLUMA_TAB_STATE_NORMAL);

		tab->priv->ask_if_externally_modified = TRUE;
			
		end_saving (tab);
	}
}

static void 
externally_modified_notification_message_area_response (GtkWidget        *message_area,
							gint              response_id,
							PlumaTab         *tab)
{
	PlumaView *view;

	set_message_area (tab, NULL);
	view = pluma_tab_get_view (tab);

	if (response_id == GTK_RESPONSE_OK)
	{
		_pluma_tab_revert (tab);
	}
	else
	{
		tab->priv->ask_if_externally_modified = FALSE;

		/* go back to normal state */
		pluma_tab_set_state (tab, PLUMA_TAB_STATE_NORMAL);
	}

	gtk_widget_grab_focus (GTK_WIDGET (view));
}

static void
display_externally_modified_notification (PlumaTab *tab)
{
	GtkWidget *message_area;
	PlumaDocument *doc;
	gchar *uri;
	gboolean document_modified;

	doc = pluma_tab_get_document (tab);
	g_return_if_fail (PLUMA_IS_DOCUMENT (doc));

	/* uri cannot be NULL, we're here because
	 * the file we're editing changed on disk */
	uri = pluma_document_get_uri (doc);
	g_return_if_fail (uri != NULL);

	document_modified = gtk_text_buffer_get_modified (GTK_TEXT_BUFFER(doc));
	message_area = pluma_externally_modified_message_area_new (uri, document_modified);
	g_free (uri);

	tab->priv->message_area = NULL;
	set_message_area (tab, message_area);
	gtk_widget_show (message_area);

	g_signal_connect (message_area,
			  "response",
			  G_CALLBACK (externally_modified_notification_message_area_response),
			  tab);
}

static gboolean
view_focused_in (GtkWidget     *widget,
                 GdkEventFocus *event,
                 PlumaTab      *tab)
{
	PlumaDocument *doc;

	g_return_val_if_fail (PLUMA_IS_TAB (tab), FALSE);

	/* we try to detect file changes only in the normal state */
	if (tab->priv->state != PLUMA_TAB_STATE_NORMAL)
	{
		return FALSE;
	}

	/* we already asked, don't bug the user again */
	if (!tab->priv->ask_if_externally_modified)
	{
		return FALSE;
	}

	doc = pluma_tab_get_document (tab);

	/* If file was never saved or is remote we do not check */
	if (!pluma_document_is_local (doc))
	{
		return FALSE;
	}

	if (_pluma_document_check_externally_modified (doc))
	{
		pluma_tab_set_state (tab, PLUMA_TAB_STATE_EXTERNALLY_MODIFIED_NOTIFICATION);

		display_externally_modified_notification (tab);

		return FALSE;
	}

	return FALSE;
}

static GMountOperation *
tab_mount_operation_factory (PlumaDocument *doc,
			     gpointer userdata)
{
	PlumaTab *tab = PLUMA_TAB (userdata);
	GtkWidget *window;

	window = gtk_widget_get_toplevel (GTK_WIDGET (tab));
	return gtk_mount_operation_new (GTK_WINDOW (window));
}

static void
pluma_tab_init (PlumaTab *tab)
{
	GtkWidget *sw;
	PlumaDocument *doc;
	PlumaLockdownMask lockdown;

	tab->priv = PLUMA_TAB_GET_PRIVATE (tab);

	tab->priv->state = PLUMA_TAB_STATE_NORMAL;

	tab->priv->not_editable = FALSE;

	tab->priv->save_flags = 0;

	tab->priv->ask_if_externally_modified = TRUE;
	
	/* Create the scrolled window */
	sw = gtk_scrolled_window_new (NULL, NULL);
	tab->priv->view_scrolled_window = sw;

	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);

	/* Manage auto save data */
	lockdown = pluma_app_get_lockdown (pluma_app_get_default ());
	tab->priv->auto_save = pluma_prefs_manager_get_auto_save () &&
			       !(lockdown & PLUMA_LOCKDOWN_SAVE_TO_DISK);
	tab->priv->auto_save = (tab->priv->auto_save != FALSE);

	tab->priv->auto_save_interval = pluma_prefs_manager_get_auto_save_interval ();
	if (tab->priv->auto_save_interval <= 0)
		tab->priv->auto_save_interval = GPM_DEFAULT_AUTO_SAVE_INTERVAL;

	/* Create the view */
	doc = pluma_document_new ();
	g_object_set_data (G_OBJECT (doc), PLUMA_TAB_KEY, tab);

	_pluma_document_set_mount_operation_factory (doc,
						     tab_mount_operation_factory,
						     tab);

	tab->priv->view = pluma_view_new (doc);
	g_object_unref (doc);
	gtk_widget_show (tab->priv->view);
	g_object_set_data (G_OBJECT (tab->priv->view), PLUMA_TAB_KEY, tab);

	gtk_box_pack_end (GTK_BOX (tab), sw, TRUE, TRUE, 0);
	gtk_container_add (GTK_CONTAINER (sw), tab->priv->view);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
					     GTK_SHADOW_IN);	
	gtk_widget_show (sw);

	g_signal_connect (doc,
			  "notify::uri",
			  G_CALLBACK (document_uri_notify_handler),
			  tab);
	g_signal_connect (doc,
			  "notify::shortname",
			  G_CALLBACK (document_shortname_notify_handler),
			  tab);
	g_signal_connect (doc,
			  "modified_changed",
			  G_CALLBACK (document_modified_changed),
			  tab);
	g_signal_connect (doc,
			  "loading",
			  G_CALLBACK (document_loading),
			  tab);
	g_signal_connect (doc,
			  "loaded",
			  G_CALLBACK (document_loaded),
			  tab);
	g_signal_connect (doc,
			  "saving",
			  G_CALLBACK (document_saving),
			  tab);
	g_signal_connect (doc,
			  "saved",
			  G_CALLBACK (document_saved),
			  tab);

	g_signal_connect_after (tab->priv->view,
				"focus-in-event",
				G_CALLBACK (view_focused_in),
				tab);

	g_signal_connect_after (tab->priv->view,
				"realize",
				G_CALLBACK (view_realized),
				tab);
}

GtkWidget *
_pluma_tab_new (void)
{
	return GTK_WIDGET (g_object_new (PLUMA_TYPE_TAB, NULL));
}

/* Whether create is TRUE, creates a new empty document if location does 
   not refer to an existing file */
GtkWidget *
_pluma_tab_new_from_uri (const gchar         *uri,
			 const PlumaEncoding *encoding,
			 gint                 line_pos,			
			 gboolean             create)
{
	PlumaTab *tab;

	g_return_val_if_fail (uri != NULL, NULL);

	tab = PLUMA_TAB (_pluma_tab_new ());

	_pluma_tab_load (tab,
			 uri,
			 encoding,
			 line_pos,
			 create);

	return GTK_WIDGET (tab);
}		

/**
 * pluma_tab_get_view:
 * @tab: a #PlumaTab
 *
 * Gets the #PlumaView inside @tab.
 *
 * Returns: the #PlumaView inside @tab
 */
PlumaView *
pluma_tab_get_view (PlumaTab *tab)
{
	return PLUMA_VIEW (tab->priv->view);
}

/**
 * pluma_tab_get_document:
 * @tab: a #PlumaTab
 *
 * Gets the #PlumaDocument associated to @tab.
 *
 * Returns: the #PlumaDocument associated to @tab
 */
PlumaDocument *
pluma_tab_get_document (PlumaTab *tab)
{
	return PLUMA_DOCUMENT (gtk_text_view_get_buffer (
					GTK_TEXT_VIEW (tab->priv->view)));
}

#define MAX_DOC_NAME_LENGTH 40

gchar *
_pluma_tab_get_name (PlumaTab *tab)
{
	PlumaDocument *doc;
	gchar *name;
	gchar *docname;
	gchar *tab_name;

	g_return_val_if_fail (PLUMA_IS_TAB (tab), NULL);

	doc = pluma_tab_get_document (tab);

	name = pluma_document_get_short_name_for_display (doc);

	/* Truncate the name so it doesn't get insanely wide. */
	docname = pluma_utils_str_middle_truncate (name, MAX_DOC_NAME_LENGTH);

	if (gtk_text_buffer_get_modified (GTK_TEXT_BUFFER (doc)))
	{
		tab_name = g_strdup_printf ("*%s", docname);
	} 
	else 
	{
 #if 0		
		if (pluma_document_get_readonly (doc)) 
		{
			tab_name = g_strdup_printf ("%s [%s]", docname, 
						/*Read only*/ _("RO"));
		} 
		else 
		{
			tab_name = g_strdup_printf ("%s", docname);
		}
#endif
		tab_name = g_strdup (docname);
	}
	
	g_free (docname);
	g_free (name);

	return tab_name;
}

gchar *
_pluma_tab_get_tooltips	(PlumaTab *tab)
{
	PlumaDocument *doc;
	gchar *tip;
	gchar *uri;
	gchar *ruri;
	gchar *ruri_markup;

	g_return_val_if_fail (PLUMA_IS_TAB (tab), NULL);

	doc = pluma_tab_get_document (tab);

	uri = pluma_document_get_uri_for_display (doc);
	g_return_val_if_fail (uri != NULL, NULL);

	ruri = 	pluma_utils_replace_home_dir_with_tilde (uri);
	g_free (uri);

	ruri_markup = g_markup_printf_escaped ("<i>%s</i>", ruri);

	switch (tab->priv->state)
	{
		gchar *content_type;
		gchar *mime_type;
		gchar *content_description;
		gchar *content_full_description; 
		gchar *encoding;
		const PlumaEncoding *enc;

		case PLUMA_TAB_STATE_LOADING_ERROR:
			tip = g_strdup_printf (_("Error opening file %s"),
					       ruri_markup);
			break;

		case PLUMA_TAB_STATE_REVERTING_ERROR:
			tip = g_strdup_printf (_("Error reverting file %s"),
					       ruri_markup);
			break;			

		case PLUMA_TAB_STATE_SAVING_ERROR:
			tip =  g_strdup_printf (_("Error saving file %s"),
						ruri_markup);
			break;			
		default:
			content_type = pluma_document_get_content_type (doc);
			mime_type = pluma_document_get_mime_type (doc);
			content_description = g_content_type_get_description (content_type);

			if (content_description == NULL)
				content_full_description = g_strdup (mime_type);
			else
				content_full_description = g_strdup_printf ("%s (%s)", 
						content_description, mime_type);

			g_free (content_type);
			g_free (mime_type);
			g_free (content_description);

			enc = pluma_document_get_encoding (doc);

			if (enc == NULL)
				encoding = g_strdup (_("Unicode (UTF-8)"));
			else
				encoding = pluma_encoding_to_string (enc);

			tip =  g_markup_printf_escaped ("<b>%s</b> %s\n\n"
						        "<b>%s</b> %s\n"
						        "<b>%s</b> %s",
						        _("Name:"), ruri,
						        _("MIME Type:"), content_full_description,
						        _("Encoding:"), encoding);

			g_free (encoding);
			g_free (content_full_description);

			break;
	}

	g_free (ruri);	
	g_free (ruri_markup);
	
	return tip;
}

static GdkPixbuf *
resize_icon (GdkPixbuf *pixbuf,
	     gint       size)
{
	gint width, height;

	width = gdk_pixbuf_get_width (pixbuf); 
	height = gdk_pixbuf_get_height (pixbuf);

	/* if the icon is larger than the nominal size, scale down */
	if (MAX (width, height) > size) 
	{
		GdkPixbuf *scaled_pixbuf;
		
		if (width > height) 
		{
			height = height * size / width;
			width = size;
		} 
		else 
		{
			width = width * size / height;
			height = size;
		}
		
		scaled_pixbuf = gdk_pixbuf_scale_simple	(pixbuf, 
							 width, 
							 height, 
							 GDK_INTERP_BILINEAR);
		g_object_unref (pixbuf);
		pixbuf = scaled_pixbuf;
	}

	return pixbuf;
}

static GdkPixbuf *
get_stock_icon (GtkIconTheme *theme, 
		const gchar  *stock,
		gint          size)
{
	GdkPixbuf *pixbuf;
	
	pixbuf = gtk_icon_theme_load_icon (theme, stock, size, 0, NULL);
	if (pixbuf == NULL)
		return NULL;
		
	return resize_icon (pixbuf, size);
}

static GdkPixbuf *
get_icon (GtkIconTheme *theme, 
	  GFile        *location,
	  gint          size)
{
	GdkPixbuf *pixbuf;
	GtkIconInfo *icon_info;
	GFileInfo *info;
	GIcon *gicon;

	if (location == NULL)
		return get_stock_icon (theme, GTK_STOCK_FILE, size);

	/* FIXME: Doing a sync stat is bad, this should be fixed */
	info = g_file_query_info (location, 
	                          G_FILE_ATTRIBUTE_STANDARD_ICON, 
	                          G_FILE_QUERY_INFO_NONE, 
	                          NULL, 
	                          NULL);
	if (info == NULL)
		return get_stock_icon (theme, GTK_STOCK_FILE, size);

	gicon = g_file_info_get_icon (info);

	if (gicon == NULL)
	{
		g_object_unref (info);
		return get_stock_icon (theme, GTK_STOCK_FILE, size);
	}

	icon_info = gtk_icon_theme_lookup_by_gicon (theme, gicon, size, 0);
	g_object_unref (info);	
	
	if (icon_info == NULL)
		return get_stock_icon (theme, GTK_STOCK_FILE, size);
	
	pixbuf = gtk_icon_info_load_icon (icon_info, NULL);
	gtk_icon_info_free (icon_info);
	
	if (pixbuf == NULL)
		return get_stock_icon (theme, GTK_STOCK_FILE, size);
		
	return resize_icon (pixbuf, size);
}

/* FIXME: add support for theme changed. I think it should be as easy as
   call g_object_notify (tab, "name") when the icon theme changes */
GdkPixbuf *
_pluma_tab_get_icon (PlumaTab *tab)
{
	GdkPixbuf *pixbuf;
	GtkIconTheme *theme;
	GdkScreen *screen;
	gint icon_size;

	g_return_val_if_fail (PLUMA_IS_TAB (tab), NULL);

	screen = gtk_widget_get_screen (GTK_WIDGET (tab));

	theme = gtk_icon_theme_get_for_screen (screen);
	g_return_val_if_fail (theme != NULL, NULL);

	gtk_icon_size_lookup_for_settings (gtk_widget_get_settings (GTK_WIDGET (tab)),
					   GTK_ICON_SIZE_MENU, 
					   NULL,
					   &icon_size);

	switch (tab->priv->state)
	{
		case PLUMA_TAB_STATE_LOADING:
			pixbuf = get_stock_icon (theme, 
						 GTK_STOCK_OPEN, 
						 icon_size);
			break;

		case PLUMA_TAB_STATE_REVERTING:
			pixbuf = get_stock_icon (theme, 
						 GTK_STOCK_REVERT_TO_SAVED, 
						 icon_size);						 
			break;

		case PLUMA_TAB_STATE_SAVING:
			pixbuf = get_stock_icon (theme, 
						 GTK_STOCK_SAVE, 
						 icon_size);
			break;

		case PLUMA_TAB_STATE_PRINTING:
			pixbuf = get_stock_icon (theme, 
						 GTK_STOCK_PRINT, 
						 icon_size);
			break;

		case PLUMA_TAB_STATE_PRINT_PREVIEWING:
		case PLUMA_TAB_STATE_SHOWING_PRINT_PREVIEW:		
			pixbuf = get_stock_icon (theme, 
						 GTK_STOCK_PRINT_PREVIEW, 
						 icon_size);
			break;

		case PLUMA_TAB_STATE_LOADING_ERROR:
		case PLUMA_TAB_STATE_REVERTING_ERROR:
		case PLUMA_TAB_STATE_SAVING_ERROR:
		case PLUMA_TAB_STATE_GENERIC_ERROR:
			pixbuf = get_stock_icon (theme, 
						 GTK_STOCK_DIALOG_ERROR, 
						 icon_size);
			break;

		case PLUMA_TAB_STATE_EXTERNALLY_MODIFIED_NOTIFICATION:
			pixbuf = get_stock_icon (theme,
						 GTK_STOCK_DIALOG_WARNING,
						 icon_size);
			break;

		default:
		{
			GFile *location;
			PlumaDocument *doc;

			doc = pluma_tab_get_document (tab);

			location = pluma_document_get_location (doc);
			pixbuf = get_icon (theme, location, icon_size);

			if (location)
				g_object_unref (location);
		}
	}

	return pixbuf;
}

/**
 * pluma_tab_get_from_document:
 * @doc: a #PlumaDocument
 *
 * Gets the #PlumaTab associated with @doc.
 *
 * Returns: the #PlumaTab associated with @doc
 */
PlumaTab *
pluma_tab_get_from_document (PlumaDocument *doc)
{
	gpointer res;
	
	g_return_val_if_fail (PLUMA_IS_DOCUMENT (doc), NULL);
	
	res = g_object_get_data (G_OBJECT (doc), PLUMA_TAB_KEY);
	
	return (res != NULL) ? PLUMA_TAB (res) : NULL;
}

void
_pluma_tab_load (PlumaTab            *tab,
		 const gchar         *uri,
		 const PlumaEncoding *encoding,
		 gint                 line_pos,
		 gboolean             create)
{
	PlumaDocument *doc;

	g_return_if_fail (PLUMA_IS_TAB (tab));
	g_return_if_fail (tab->priv->state == PLUMA_TAB_STATE_NORMAL);

	doc = pluma_tab_get_document (tab);
	g_return_if_fail (PLUMA_IS_DOCUMENT (doc));

	pluma_tab_set_state (tab, PLUMA_TAB_STATE_LOADING);

	tab->priv->tmp_line_pos = line_pos;
	tab->priv->tmp_encoding = encoding;

	if (tab->priv->auto_save_timeout > 0)
		remove_auto_save_timeout (tab);

	pluma_document_load (doc,
			     uri,
			     encoding,
			     line_pos,
			     create);
}

void
_pluma_tab_revert (PlumaTab *tab)
{
	PlumaDocument *doc;
	gchar *uri;

	g_return_if_fail (PLUMA_IS_TAB (tab));
	g_return_if_fail ((tab->priv->state == PLUMA_TAB_STATE_NORMAL) ||
			  (tab->priv->state == PLUMA_TAB_STATE_EXTERNALLY_MODIFIED_NOTIFICATION));

	if (tab->priv->state == PLUMA_TAB_STATE_EXTERNALLY_MODIFIED_NOTIFICATION)
	{
		set_message_area (tab, NULL);
	}

	doc = pluma_tab_get_document (tab);
	g_return_if_fail (PLUMA_IS_DOCUMENT (doc));

	pluma_tab_set_state (tab, PLUMA_TAB_STATE_REVERTING);

	uri = pluma_document_get_uri (doc);
	g_return_if_fail (uri != NULL);

	tab->priv->tmp_line_pos = 0;
	tab->priv->tmp_encoding = pluma_document_get_encoding (doc);

	if (tab->priv->auto_save_timeout > 0)
		remove_auto_save_timeout (tab);

	pluma_document_load (doc,
			     uri,
			     tab->priv->tmp_encoding,
			     0,
			     FALSE);

	g_free (uri);
}

void
_pluma_tab_save (PlumaTab *tab)
{
	PlumaDocument *doc;
	PlumaDocumentSaveFlags save_flags;

	g_return_if_fail (PLUMA_IS_TAB (tab));
	g_return_if_fail ((tab->priv->state == PLUMA_TAB_STATE_NORMAL) ||
			  (tab->priv->state == PLUMA_TAB_STATE_EXTERNALLY_MODIFIED_NOTIFICATION) ||
			  (tab->priv->state == PLUMA_TAB_STATE_SHOWING_PRINT_PREVIEW));
	g_return_if_fail (tab->priv->tmp_save_uri == NULL);
	g_return_if_fail (tab->priv->tmp_encoding == NULL);

	doc = pluma_tab_get_document (tab);
	g_return_if_fail (PLUMA_IS_DOCUMENT (doc));
	g_return_if_fail (!pluma_document_is_untitled (doc));

	if (tab->priv->state == PLUMA_TAB_STATE_EXTERNALLY_MODIFIED_NOTIFICATION)
	{
		/* We already told the user about the external
		 * modification: hide the message area and set
		 * the save flag.
		 */

		set_message_area (tab, NULL);
		save_flags = tab->priv->save_flags | PLUMA_DOCUMENT_SAVE_IGNORE_MTIME;
	}
	else
	{
		save_flags = tab->priv->save_flags;
	}

	pluma_tab_set_state (tab, PLUMA_TAB_STATE_SAVING);

	/* uri used in error messages, will be freed in document_saved */
	tab->priv->tmp_save_uri = pluma_document_get_uri (doc);
	tab->priv->tmp_encoding = pluma_document_get_encoding (doc); 

	if (tab->priv->auto_save_timeout > 0)
		remove_auto_save_timeout (tab);
		
	pluma_document_save (doc, save_flags);
}

static gboolean
pluma_tab_auto_save (PlumaTab *tab)
{
	PlumaDocument *doc;

	pluma_debug (DEBUG_TAB);
	
	g_return_val_if_fail (tab->priv->tmp_save_uri == NULL, FALSE);
	g_return_val_if_fail (tab->priv->tmp_encoding == NULL, FALSE);
	
	doc = pluma_tab_get_document (tab);
	
	g_return_val_if_fail (!pluma_document_is_untitled (doc), FALSE);
	g_return_val_if_fail (!pluma_document_get_readonly (doc), FALSE);

	g_return_val_if_fail (tab->priv->auto_save_timeout > 0, FALSE);
	g_return_val_if_fail (tab->priv->auto_save, FALSE);
	g_return_val_if_fail (tab->priv->auto_save_interval > 0, FALSE);

	if (!gtk_text_buffer_get_modified (GTK_TEXT_BUFFER(doc)))
	{
		pluma_debug_message (DEBUG_TAB, "Document not modified");

		return TRUE;
	}
			
	if ((tab->priv->state != PLUMA_TAB_STATE_NORMAL) &&
	    (tab->priv->state != PLUMA_TAB_STATE_SHOWING_PRINT_PREVIEW))
	{
		/* Retry after 30 seconds */
		guint timeout;

		pluma_debug_message (DEBUG_TAB, "Retry after 30 seconds");

		/* Add a new timeout */
		timeout = g_timeout_add_seconds (30,
						 (GSourceFunc) pluma_tab_auto_save,
						 tab);

		tab->priv->auto_save_timeout = timeout;

	    	/* Returns FALSE so the old timeout is "destroyed" */
		return FALSE;
	}
	
	pluma_tab_set_state (tab, PLUMA_TAB_STATE_SAVING);

	/* uri used in error messages, will be freed in document_saved */
	tab->priv->tmp_save_uri = pluma_document_get_uri (doc);
	tab->priv->tmp_encoding = pluma_document_get_encoding (doc); 

	/* Set auto_save_timeout to 0 since the timeout is going to be destroyed */
	tab->priv->auto_save_timeout = 0;

	/* Since we are autosaving, we need to preserve the backup that was produced
	   the last time the user "manually" saved the file. In the case a recoverable
	   error happens while saving, the last backup is not preserved since the user
	   expressed his willing of saving the file */
	pluma_document_save (doc, tab->priv->save_flags | PLUMA_DOCUMENT_SAVE_PRESERVE_BACKUP);
	
	pluma_debug_message (DEBUG_TAB, "Done");
	
	/* Returns FALSE so the old timeout is "destroyed" */
	return FALSE;
}

void
_pluma_tab_save_as (PlumaTab                 *tab,
                    const gchar              *uri,
                    const PlumaEncoding      *encoding,
                    PlumaDocumentNewlineType  newline_type)
{
	PlumaDocument *doc;
	PlumaDocumentSaveFlags save_flags;

	g_return_if_fail (PLUMA_IS_TAB (tab));
	g_return_if_fail ((tab->priv->state == PLUMA_TAB_STATE_NORMAL) ||
			  (tab->priv->state == PLUMA_TAB_STATE_EXTERNALLY_MODIFIED_NOTIFICATION) ||
			  (tab->priv->state == PLUMA_TAB_STATE_SHOWING_PRINT_PREVIEW));
	g_return_if_fail (encoding != NULL);

	g_return_if_fail (tab->priv->tmp_save_uri == NULL);
	g_return_if_fail (tab->priv->tmp_encoding == NULL);

	doc = pluma_tab_get_document (tab);
	g_return_if_fail (PLUMA_IS_DOCUMENT (doc));

	/* reset the save flags, when saving as */
	tab->priv->save_flags = 0;

	if (tab->priv->state == PLUMA_TAB_STATE_EXTERNALLY_MODIFIED_NOTIFICATION)
	{
		/* We already told the user about the external
		 * modification: hide the message area and set
		 * the save flag.
		 */

		set_message_area (tab, NULL);
		save_flags = tab->priv->save_flags | PLUMA_DOCUMENT_SAVE_IGNORE_MTIME;
	}
	else
	{
		save_flags = tab->priv->save_flags;
	}

	pluma_tab_set_state (tab, PLUMA_TAB_STATE_SAVING);

	/* uri used in error messages... strdup because errors are async
	 * and the string can go away, will be freed in document_saved */
	tab->priv->tmp_save_uri = g_strdup (uri);
	tab->priv->tmp_encoding = encoding;

	if (tab->priv->auto_save_timeout > 0)
		remove_auto_save_timeout (tab);

	/* FIXME: this should behave the same as encoding, setting it here
	   makes it persistent (if save fails, it's remembered). It's not
	   a very big deal, but would be nice to have them follow the
	   same pattern. This can be changed once we break API for 3.0 */
	pluma_document_set_newline_type (doc, newline_type);
	pluma_document_save_as (doc, uri, encoding, tab->priv->save_flags);
}

#define PLUMA_PAGE_SETUP_KEY "pluma-page-setup-key"
#define PLUMA_PRINT_SETTINGS_KEY "pluma-print-settings-key"

static GtkPageSetup *
get_page_setup (PlumaTab *tab)
{
	gpointer data;
	PlumaDocument *doc;

	doc = pluma_tab_get_document (tab);

	data = g_object_get_data (G_OBJECT (doc),
				  PLUMA_PAGE_SETUP_KEY);

	if (data == NULL)
	{
		return _pluma_app_get_default_page_setup (pluma_app_get_default());
	}
	else
	{
		return gtk_page_setup_copy (GTK_PAGE_SETUP (data));
	}
}

static GtkPrintSettings *
get_print_settings (PlumaTab *tab)
{
	gpointer data;
	PlumaDocument *doc;
	GtkPrintSettings *settings;
	gchar *uri, *name;

	doc = pluma_tab_get_document (tab);

	data = g_object_get_data (G_OBJECT (doc),
				  PLUMA_PRINT_SETTINGS_KEY);

	if (data == NULL)
	{
		settings = _pluma_app_get_default_print_settings (pluma_app_get_default());
	}
	else
	{
		settings = gtk_print_settings_copy (GTK_PRINT_SETTINGS (data));
	}

	name = pluma_document_get_short_name_for_display (doc);
	uri = g_strconcat ("file://",
			   g_get_user_special_dir (G_USER_DIRECTORY_DOCUMENTS),
			   "/", name, ".pdf", NULL);

	gtk_print_settings_set (settings, GTK_PRINT_SETTINGS_OUTPUT_URI, uri);

	g_free (uri);
	g_free (name);

	return settings;
}

/* FIXME: show the message area only if the operation will be "long" */
static void
printing_cb (PlumaPrintJob       *job,
	     PlumaPrintJobStatus  status,
	     PlumaTab            *tab)
{
	g_return_if_fail (PLUMA_IS_PROGRESS_MESSAGE_AREA (tab->priv->message_area));	

	gtk_widget_show (tab->priv->message_area);

	pluma_progress_message_area_set_text (PLUMA_PROGRESS_MESSAGE_AREA (tab->priv->message_area),
					      pluma_print_job_get_status_string (job));

	pluma_progress_message_area_set_fraction (PLUMA_PROGRESS_MESSAGE_AREA (tab->priv->message_area),
						  pluma_print_job_get_progress (job));
}

static void
store_print_settings (PlumaTab      *tab,
		      PlumaPrintJob *job)
{
	PlumaDocument *doc;
	GtkPrintSettings *settings;
	GtkPageSetup *page_setup;

	doc = pluma_tab_get_document (tab);

	settings = pluma_print_job_get_print_settings (job);

	/* clear n-copies settings since we do not want to
	 * persist that one */
	gtk_print_settings_unset (settings,
				  GTK_PRINT_SETTINGS_N_COPIES);

	/* remember settings for this document */
	g_object_set_data_full (G_OBJECT (doc),
				PLUMA_PRINT_SETTINGS_KEY,
				g_object_ref (settings),
				(GDestroyNotify)g_object_unref);

	/* make them the default */
	_pluma_app_set_default_print_settings (pluma_app_get_default (),
					       settings);

	page_setup = pluma_print_job_get_page_setup (job);

	/* remember page setup for this document */
	g_object_set_data_full (G_OBJECT (doc),
				PLUMA_PAGE_SETUP_KEY,
				g_object_ref (page_setup),
				(GDestroyNotify)g_object_unref);

	/* make it the default */
	_pluma_app_set_default_page_setup (pluma_app_get_default (),
					   page_setup);
}

static void
done_printing_cb (PlumaPrintJob       *job,
		  PlumaPrintJobResult  result,
		  const GError        *error,
		  PlumaTab            *tab)
{
	PlumaView *view;

	g_return_if_fail (tab->priv->state == PLUMA_TAB_STATE_PRINT_PREVIEWING ||
			  tab->priv->state == PLUMA_TAB_STATE_SHOWING_PRINT_PREVIEW ||
			  tab->priv->state == PLUMA_TAB_STATE_PRINTING);

	if (tab->priv->state == PLUMA_TAB_STATE_SHOWING_PRINT_PREVIEW)
	{
		/* print preview has been destroyed... */
		tab->priv->print_preview = NULL;
	}
	else
	{
		g_return_if_fail (PLUMA_IS_PROGRESS_MESSAGE_AREA (tab->priv->message_area));

		set_message_area (tab, NULL); /* destroy the message area */
	}

	// TODO: check status and error

	if (result ==  PLUMA_PRINT_JOB_RESULT_OK)
	{
		store_print_settings (tab, job);
	}

#if 0
	if (tab->priv->print_preview != NULL)
	{
		/* If we were printing while showing the print preview,
		   see bug #352658 */
		gtk_widget_destroy (tab->priv->print_preview);
		g_return_if_fail (tab->priv->state == PLUMA_TAB_STATE_PRINTING);
	}
#endif

	pluma_tab_set_state (tab, PLUMA_TAB_STATE_NORMAL);

	view = pluma_tab_get_view (tab);
	gtk_widget_grab_focus (GTK_WIDGET (view));

 	g_object_unref (tab->priv->print_job);
	tab->priv->print_job = NULL;
}

#if 0
static void
print_preview_destroyed (GtkWidget *preview,
			 PlumaTab  *tab)
{
	tab->priv->print_preview = NULL;

	if (tab->priv->state == PLUMA_TAB_STATE_SHOWING_PRINT_PREVIEW)
	{
		PlumaView *view;

		pluma_tab_set_state (tab, PLUMA_TAB_STATE_NORMAL);

		view = pluma_tab_get_view (tab);
		gtk_widget_grab_focus (GTK_WIDGET (view));
	}
	else
	{
		/* This should happen only when printing while showing the print
		 * preview. In this case let us continue whithout changing
		 * the state and show the document. See bug #352658 */
		gtk_widget_show (tab->priv->view_scrolled_window);
		
		g_return_if_fail (tab->priv->state == PLUMA_TAB_STATE_PRINTING);
	}	
}
#endif

static void
show_preview_cb (PlumaPrintJob       *job,
		 PlumaPrintPreview   *preview,
		 PlumaTab            *tab)
{
//	g_return_if_fail (tab->priv->state == PLUMA_TAB_STATE_PRINT_PREVIEWING);
	g_return_if_fail (tab->priv->print_preview == NULL);

	set_message_area (tab, NULL); /* destroy the message area */

	tab->priv->print_preview = GTK_WIDGET (preview);
	gtk_box_pack_end (GTK_BOX (tab),
			  tab->priv->print_preview,
			  TRUE,
			  TRUE,
			  0);
	gtk_widget_show (tab->priv->print_preview);
	gtk_widget_grab_focus (tab->priv->print_preview);

/* when the preview gets destroyed we get "done" signal
	g_signal_connect (tab->priv->print_preview,
			  "destroy",
			  G_CALLBACK (print_preview_destroyed),
			  tab);	
*/
	pluma_tab_set_state (tab, PLUMA_TAB_STATE_SHOWING_PRINT_PREVIEW);
}

#if 0

static void
set_print_preview (PlumaTab  *tab,
		   GtkWidget *print_preview)
{
	if (tab->priv->print_preview == print_preview)
		return;
		
	if (tab->priv->print_preview != NULL)
		gtk_widget_destroy (tab->priv->print_preview);

	tab->priv->print_preview = print_preview;

	gtk_box_pack_end (GTK_BOX (tab),
			  tab->priv->print_preview,
			  TRUE,
			  TRUE,
			  0);		

	gtk_widget_grab_focus (tab->priv->print_preview);

	g_signal_connect (tab->priv->print_preview,
			  "destroy",
			  G_CALLBACK (print_preview_destroyed),
			  tab);
}

static void
preview_finished_cb (GtkSourcePrintJob *pjob, PlumaTab *tab)
{
	MatePrintJob *gjob;
	GtkWidget *preview = NULL;

	g_return_if_fail (PLUMA_IS_PROGRESS_MESSAGE_AREA (tab->priv->message_area));
	set_message_area (tab, NULL); /* destroy the message area */
	
	gjob = gtk_source_print_job_get_print_job (pjob);

	preview = pluma_print_job_preview_new (gjob);	
 	g_object_unref (gjob);
	
	set_print_preview (tab, preview);
	
	gtk_widget_show (preview);
	g_object_unref (pjob);
	
	pluma_tab_set_state (tab, PLUMA_TAB_STATE_SHOWING_PRINT_PREVIEW);
}


#endif

static void
print_cancelled (GtkWidget        *area,
                 gint              response_id,
                 PlumaTab         *tab)
{
	g_return_if_fail (PLUMA_IS_PROGRESS_MESSAGE_AREA (tab->priv->message_area));

	pluma_print_job_cancel (tab->priv->print_job);

	g_debug ("print_cancelled");
}

static void
show_printing_message_area (PlumaTab *tab, gboolean preview)
{
	GtkWidget *area;

	if (preview)
		area = pluma_progress_message_area_new (GTK_STOCK_PRINT_PREVIEW,
							"",
							TRUE);
	else
		area = pluma_progress_message_area_new (GTK_STOCK_PRINT,
							"",
							TRUE);

	g_signal_connect (area,
			  "response",
			  G_CALLBACK (print_cancelled),
			  tab);
	  
	set_message_area (tab, area);
}

#if !GTK_CHECK_VERSION (2, 17, 4)

static void
page_setup_done_cb (GtkPageSetup *setup,
		    PlumaTab     *tab)
{
	if (setup != NULL)
	{
		PlumaDocument *doc;

		doc = pluma_tab_get_document (tab);

		/* remember it for this document */
		g_object_set_data_full (G_OBJECT (doc),
					PLUMA_PAGE_SETUP_KEY,
					g_object_ref (setup),
					(GDestroyNotify)g_object_unref);

		/* make it the default */
		_pluma_app_set_default_page_setup (pluma_app_get_default(),
						   setup);
	}
}

void 
_pluma_tab_page_setup (PlumaTab *tab)
{
	GtkPageSetup *setup;
	GtkPrintSettings *settings;

	g_return_if_fail (PLUMA_IS_TAB (tab));

	setup = get_page_setup (tab);
	settings = get_print_settings (tab);

	gtk_print_run_page_setup_dialog_async (GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (tab))),
		 			       setup,
		 			       settings,
					       (GtkPageSetupDoneFunc) page_setup_done_cb,
					       tab);

	/* CHECK: should we unref setup and settings? */
}

#endif

static void
pluma_tab_print_or_print_preview (PlumaTab                *tab,
				  GtkPrintOperationAction  print_action)
{
	PlumaView *view;
	gboolean is_preview;
	GtkPageSetup *setup;
	GtkPrintSettings *settings;
	GtkPrintOperationResult res;
	GError *error = NULL;

	g_return_if_fail (tab->priv->print_job == NULL);
	g_return_if_fail (tab->priv->state == PLUMA_TAB_STATE_NORMAL);

	view = pluma_tab_get_view (tab);

	is_preview = (print_action == GTK_PRINT_OPERATION_ACTION_PREVIEW);

	tab->priv->print_job = pluma_print_job_new (view);
	g_object_add_weak_pointer (G_OBJECT (tab->priv->print_job), 
				   (gpointer *) &tab->priv->print_job);

	show_printing_message_area (tab, is_preview);

	g_signal_connect (tab->priv->print_job,
			  "printing",
			  G_CALLBACK (printing_cb),
			  tab);
	g_signal_connect (tab->priv->print_job,
			  "show-preview",
			  G_CALLBACK (show_preview_cb),
			  tab);
	g_signal_connect (tab->priv->print_job,
			  "done",
			  G_CALLBACK (done_printing_cb),
			  tab);

	if (is_preview)
		pluma_tab_set_state (tab, PLUMA_TAB_STATE_PRINT_PREVIEWING);
	else
		pluma_tab_set_state (tab, PLUMA_TAB_STATE_PRINTING);

	setup = get_page_setup (tab);
	settings = get_print_settings (tab);

	res = pluma_print_job_print (tab->priv->print_job,
				     print_action,
				     setup,
				     settings,
				     GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (tab))),
				     &error);

	// TODO: manage res in the correct way
	if (res == GTK_PRINT_OPERATION_RESULT_ERROR)
	{
		/* FIXME: go in error state */
		pluma_tab_set_state (tab, PLUMA_TAB_STATE_NORMAL);
		g_warning ("Async print preview failed (%s)", error->message);
		g_object_unref (tab->priv->print_job);
		g_error_free (error);
	}
}

void 
_pluma_tab_print (PlumaTab     *tab)
{
	g_return_if_fail (PLUMA_IS_TAB (tab));

	/* FIXME: currently we can have just one printoperation going on
	 * at a given time, so before starting the print we close the preview.
	 * Would be nice to handle it properly though */
	if (tab->priv->state == PLUMA_TAB_STATE_SHOWING_PRINT_PREVIEW)
	{
		gtk_widget_destroy (tab->priv->print_preview);
	}

	pluma_tab_print_or_print_preview (tab,
					  GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG);
}

void
_pluma_tab_print_preview (PlumaTab     *tab)
{
	g_return_if_fail (PLUMA_IS_TAB (tab));

	pluma_tab_print_or_print_preview (tab,
					  GTK_PRINT_OPERATION_ACTION_PREVIEW);
}

void 
_pluma_tab_mark_for_closing (PlumaTab *tab)
{
	g_return_if_fail (PLUMA_IS_TAB (tab));
	g_return_if_fail (tab->priv->state == PLUMA_TAB_STATE_NORMAL);
	
	pluma_tab_set_state (tab, PLUMA_TAB_STATE_CLOSING);
}

gboolean
_pluma_tab_can_close (PlumaTab *tab)
{
	PlumaDocument *doc;
	PlumaTabState  ts;

	g_return_val_if_fail (PLUMA_IS_TAB (tab), FALSE);

	ts = pluma_tab_get_state (tab);

	/* if we are loading or reverting, the tab can be closed */
	if ((ts == PLUMA_TAB_STATE_LOADING)       ||
	    (ts == PLUMA_TAB_STATE_LOADING_ERROR) ||
	    (ts == PLUMA_TAB_STATE_REVERTING)     ||
	    (ts == PLUMA_TAB_STATE_REVERTING_ERROR)) /* CHECK: I'm not sure this is the right behavior for REVERTING ERROR */
		return TRUE;

	/* Do not close tab with saving errors */
	if (ts == PLUMA_TAB_STATE_SAVING_ERROR)
		return FALSE;
		
	doc = pluma_tab_get_document (tab);

	/* TODO: we need to save the file also if it has been externally
	   modified - Paolo (Oct 10, 2005) */

	return (!gtk_text_buffer_get_modified (GTK_TEXT_BUFFER (doc)) &&
		!pluma_document_get_deleted (doc));
}

/**
 * pluma_tab_get_auto_save_enabled:
 * @tab: a #PlumaTab
 * 
 * Gets the current state for the autosave feature
 * 
 * Return value: %TRUE if the autosave is enabled, else %FALSE
 **/
gboolean
pluma_tab_get_auto_save_enabled	(PlumaTab *tab)
{
	pluma_debug (DEBUG_TAB);

	g_return_val_if_fail (PLUMA_IS_TAB (tab), FALSE);

	return tab->priv->auto_save;
}

/**
 * pluma_tab_set_auto_save_enabled:
 * @tab: a #PlumaTab
 * @enable: enable (%TRUE) or disable (%FALSE) auto save
 * 
 * Enables or disables the autosave feature. It does not install an
 * autosave timeout if the document is new or is read-only
 **/
void
pluma_tab_set_auto_save_enabled	(PlumaTab *tab, 
				 gboolean enable)
{
	PlumaDocument *doc = NULL;
	PlumaLockdownMask lockdown;
	
	pluma_debug (DEBUG_TAB);

	g_return_if_fail (PLUMA_IS_TAB (tab));

	/* Force disabling when lockdown is active */
	lockdown = pluma_app_get_lockdown (pluma_app_get_default());
	if (lockdown & PLUMA_LOCKDOWN_SAVE_TO_DISK)
		enable = FALSE;
	
	doc = pluma_tab_get_document (tab);

	if (tab->priv->auto_save == enable)
		return;

	tab->priv->auto_save = enable;

 	if (enable && 
 	    (tab->priv->auto_save_timeout <=0) &&
 	    !pluma_document_is_untitled (doc) &&
 	    !pluma_document_get_readonly (doc))
 	{
 		if ((tab->priv->state != PLUMA_TAB_STATE_LOADING) &&
		    (tab->priv->state != PLUMA_TAB_STATE_SAVING) &&
		    (tab->priv->state != PLUMA_TAB_STATE_REVERTING) &&
		    (tab->priv->state != PLUMA_TAB_STATE_LOADING_ERROR) &&
		    (tab->priv->state != PLUMA_TAB_STATE_SAVING_ERROR) &&
		    (tab->priv->state != PLUMA_TAB_STATE_REVERTING_ERROR))
		{
			install_auto_save_timeout (tab);
		}
		/* else: the timeout will be installed when loading/saving/reverting
		         will terminate */
		
		return;
	}
 		
 	if (!enable && (tab->priv->auto_save_timeout > 0))
 	{
		remove_auto_save_timeout (tab);
		
 		return; 
 	} 

 	g_return_if_fail ((!enable && (tab->priv->auto_save_timeout <= 0)) ||  
 			  pluma_document_is_untitled (doc) || pluma_document_get_readonly (doc)); 
}

/**
 * pluma_tab_get_auto_save_interval:
 * @tab: a #PlumaTab
 * 
 * Gets the current interval for the autosaves
 * 
 * Return value: the value of the autosave
 **/
gint 
pluma_tab_get_auto_save_interval (PlumaTab *tab)
{
	pluma_debug (DEBUG_TAB);

	g_return_val_if_fail (PLUMA_IS_TAB (tab), 0);

	return tab->priv->auto_save_interval;
}

/**
 * pluma_tab_set_auto_save_interval:
 * @tab: a #PlumaTab
 * @interval: the new interval
 * 
 * Sets the interval for the autosave feature. It does nothing if the
 * interval is the same as the one already present. It removes the old
 * interval timeout and adds a new one with the autosave passed as
 * argument.
 **/
void 
pluma_tab_set_auto_save_interval (PlumaTab *tab, 
				  gint interval)
{
	PlumaDocument *doc = NULL;
	
	pluma_debug (DEBUG_TAB);
	
	g_return_if_fail (PLUMA_IS_TAB (tab));

	doc = pluma_tab_get_document(tab);

	g_return_if_fail (PLUMA_IS_DOCUMENT (doc));
	g_return_if_fail (interval > 0);

	if (tab->priv->auto_save_interval == interval)
		return;

	tab->priv->auto_save_interval = interval;
		
	if (!tab->priv->auto_save)
		return;

	if (tab->priv->auto_save_timeout > 0)
	{
		g_return_if_fail (!pluma_document_is_untitled (doc));
		g_return_if_fail (!pluma_document_get_readonly (doc));

		remove_auto_save_timeout (tab);

		install_auto_save_timeout (tab);
	}
}

void
pluma_tab_set_info_bar (PlumaTab  *tab,
                        GtkWidget *info_bar)
{
	g_return_if_fail (PLUMA_IS_TAB (tab));
	g_return_if_fail (info_bar == NULL || GTK_IS_WIDGET (info_bar));

	/* FIXME: this can cause problems with the tab state machine */
	set_message_area (tab, info_bar);
}
