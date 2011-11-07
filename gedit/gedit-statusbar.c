/*
 * gedit-statusbar.c
 * This file is part of gedit
 *
 * Copyright (C) 2005 - Paolo Borelli
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

#include <string.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "gedit-statusbar.h"

#define GEDIT_STATUSBAR_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    GEDIT_TYPE_STATUSBAR,\
					    GeditStatusbarPrivate))

struct _GeditStatusbarPrivate
{
	GtkWidget     *overwrite_mode_statusbar;
	GtkWidget     *cursor_position_statusbar;

	GtkWidget     *state_frame;
	GtkWidget     *load_image;
	GtkWidget     *save_image;
	GtkWidget     *print_image;

	GtkWidget     *error_frame;
	GtkWidget     *error_event_box;

	/* tmp flash timeout data */
	guint          flash_timeout;
	guint          flash_context_id;
	guint          flash_message_id;
};

G_DEFINE_TYPE(GeditStatusbar, gedit_statusbar, GTK_TYPE_STATUSBAR)


static gchar *
get_overwrite_mode_string (gboolean overwrite)
{
	return g_strconcat ("  ", overwrite ? _("OVR") :  _("INS"), NULL);
}

static gint
get_overwrite_mode_length (void)
{
	return 2 + MAX (g_utf8_strlen (_("OVR"), -1), g_utf8_strlen (_("INS"), -1));
}

static void
gedit_statusbar_notify (GObject    *object,
			GParamSpec *pspec)
{
	/* don't allow gtk_statusbar_set_has_resize_grip to mess with us.
	 * See _gedit_statusbar_set_has_resize_grip for an explanation.
	 */
	if (strcmp (g_param_spec_get_name (pspec), "has-resize-grip") == 0)
	{
		gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (object), FALSE);
		return;
	}

	if (G_OBJECT_CLASS (gedit_statusbar_parent_class)->notify)
		G_OBJECT_CLASS (gedit_statusbar_parent_class)->notify (object, pspec);
}

static void
gedit_statusbar_finalize (GObject *object)
{
	GeditStatusbar *statusbar = GEDIT_STATUSBAR (object);

	if (statusbar->priv->flash_timeout > 0)
		g_source_remove (statusbar->priv->flash_timeout);

	G_OBJECT_CLASS (gedit_statusbar_parent_class)->finalize (object);
}

static void
gedit_statusbar_class_init (GeditStatusbarClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->notify = gedit_statusbar_notify;
	object_class->finalize = gedit_statusbar_finalize;

	g_type_class_add_private (object_class, sizeof (GeditStatusbarPrivate));
}

#define RESIZE_GRIP_EXTRA_WIDTH 30

static void
set_statusbar_width_chars (GtkWidget *statusbar,
			   gint       n_chars,
			   gboolean   has_resize_grip)
{
	PangoContext *context;
	PangoFontMetrics *metrics;
	gint char_width, digit_width, width;
	GtkStyle *style;

	context = gtk_widget_get_pango_context (statusbar);
	style = gtk_widget_get_style (GTK_WIDGET (statusbar));
	metrics = pango_context_get_metrics (context,
					     style->font_desc,
					     pango_context_get_language (context));

	char_width = pango_font_metrics_get_approximate_digit_width (metrics);
	digit_width = pango_font_metrics_get_approximate_char_width (metrics);

	width = PANGO_PIXELS (MAX (char_width, digit_width) * n_chars);

	pango_font_metrics_unref (metrics);

	/* If there is a resize grip, allocate some extra width.
	 * It would be nice to calculate the exact size programmatically
	 * but I could not find out how to do it */
	if (has_resize_grip)
		width += RESIZE_GRIP_EXTRA_WIDTH;

	gtk_widget_set_size_request (statusbar, width, -1);
}

static void
gedit_statusbar_init (GeditStatusbar *statusbar)
{
	GtkWidget *hbox;
	GtkWidget *error_image;

	statusbar->priv = GEDIT_STATUSBAR_GET_PRIVATE (statusbar);

	gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (statusbar), FALSE);

	statusbar->priv->overwrite_mode_statusbar = gtk_statusbar_new ();
	gtk_widget_show (statusbar->priv->overwrite_mode_statusbar);
	gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (statusbar->priv->overwrite_mode_statusbar),
					   TRUE);
	set_statusbar_width_chars (statusbar->priv->overwrite_mode_statusbar,
				   get_overwrite_mode_length (),
				   TRUE);
	gtk_box_pack_end (GTK_BOX (statusbar),
			  statusbar->priv->overwrite_mode_statusbar,
			  FALSE, TRUE, 0);

	statusbar->priv->cursor_position_statusbar = gtk_statusbar_new ();
	gtk_widget_show (statusbar->priv->cursor_position_statusbar);
	gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (statusbar->priv->cursor_position_statusbar),
					   FALSE);
	set_statusbar_width_chars (statusbar->priv->cursor_position_statusbar, 18, FALSE);
	gtk_box_pack_end (GTK_BOX (statusbar),
			  statusbar->priv->cursor_position_statusbar,
			  FALSE, TRUE, 0);

	statusbar->priv->state_frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (statusbar->priv->state_frame), GTK_SHADOW_IN);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (statusbar->priv->state_frame), hbox);

	statusbar->priv->load_image = gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
	statusbar->priv->save_image = gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
	statusbar->priv->print_image = gtk_image_new_from_stock (GTK_STOCK_PRINT, GTK_ICON_SIZE_MENU);

	gtk_widget_show (hbox);

	gtk_box_pack_start (GTK_BOX (hbox),
			    statusbar->priv->load_image,
			    FALSE, TRUE, 4);
	gtk_box_pack_start (GTK_BOX (hbox),
			    statusbar->priv->save_image,
			    FALSE, TRUE, 4);
	gtk_box_pack_start (GTK_BOX (hbox),
			    statusbar->priv->print_image,
			    FALSE, TRUE, 4);

	gtk_box_pack_start (GTK_BOX (statusbar),
			    statusbar->priv->state_frame,
			    FALSE, TRUE, 0);

	statusbar->priv->error_frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (statusbar->priv->error_frame), GTK_SHADOW_IN);

	error_image = gtk_image_new_from_stock (GTK_STOCK_DIALOG_ERROR, GTK_ICON_SIZE_MENU);
	gtk_misc_set_padding (GTK_MISC (error_image), 4, 0);
	gtk_widget_show (error_image);

	statusbar->priv->error_event_box = gtk_event_box_new ();
	gtk_event_box_set_visible_window  (GTK_EVENT_BOX (statusbar->priv->error_event_box),
					   FALSE);
	gtk_widget_show (statusbar->priv->error_event_box);

	gtk_container_add (GTK_CONTAINER (statusbar->priv->error_frame),
			   statusbar->priv->error_event_box);
	gtk_container_add (GTK_CONTAINER (statusbar->priv->error_event_box),
			   error_image);

	gtk_box_pack_start (GTK_BOX (statusbar),
			    statusbar->priv->error_frame,
			    FALSE, TRUE, 0);

	gtk_box_reorder_child (GTK_BOX (statusbar),
			       statusbar->priv->error_frame,
			       0);
}

/**
 * gedit_statusbar_new:
 *
 * Creates a new #GeditStatusbar.
 *
 * Return value: the new #GeditStatusbar object
 **/
GtkWidget *
gedit_statusbar_new (void)
{
	return GTK_WIDGET (g_object_new (GEDIT_TYPE_STATUSBAR, NULL));
}

/**
 * gedit_set_has_resize_grip:
 * @statusbar: a #GeditStatusbar
 * @show: if the resize grip is shown
 *
 * Sets if a resize grip showld be shown.
 *
 **/
 /*
  * I don't like this much, in a perfect world it would have been
  * possible to override the parent property and use
  * gtk_statusbar_set_has_resize_grip. Unfortunately this is not
  * possible and it's not even possible to intercept the notify signal
  * since the parent property should always be set to false thus when
  * using set_resize_grip (FALSE) the property doesn't change and the
  * notification is not emitted.
  * For now just add this private method; if needed we can turn it into
  * a property.
  */
void
_gedit_statusbar_set_has_resize_grip (GeditStatusbar *bar,
				      gboolean        show)
{
	g_return_if_fail (GEDIT_IS_STATUSBAR (bar));

	gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (bar->priv->overwrite_mode_statusbar),
					   show);
}

/**
 * gedit_statusbar_set_overwrite:
 * @statusbar: a #GeditStatusbar
 * @overwrite: if the overwrite mode is set
 *
 * Sets the overwrite mode on the statusbar.
 **/
void
gedit_statusbar_set_overwrite (GeditStatusbar *statusbar,
                               gboolean        overwrite)
{
	gchar *msg;

	g_return_if_fail (GEDIT_IS_STATUSBAR (statusbar));

	gtk_statusbar_pop (GTK_STATUSBAR (statusbar->priv->overwrite_mode_statusbar), 0);

	msg = get_overwrite_mode_string (overwrite);

	gtk_statusbar_push (GTK_STATUSBAR (statusbar->priv->overwrite_mode_statusbar), 0, msg);

      	g_free (msg);
}

void
gedit_statusbar_clear_overwrite (GeditStatusbar *statusbar)
{
	g_return_if_fail (GEDIT_IS_STATUSBAR (statusbar));

	gtk_statusbar_pop (GTK_STATUSBAR (statusbar->priv->overwrite_mode_statusbar), 0);
}

/**
 * gedit_statusbar_cursor_position:
 * @statusbar: an #GeditStatusbar
 * @line: line position
 * @col: column position
 *
 * Sets the cursor position on the statusbar.
 **/
void
gedit_statusbar_set_cursor_position (GeditStatusbar *statusbar,
				     gint            line,
				     gint            col)
{
	gchar *msg;

	g_return_if_fail (GEDIT_IS_STATUSBAR (statusbar));

	gtk_statusbar_pop (GTK_STATUSBAR (statusbar->priv->cursor_position_statusbar), 0);

	if ((line == -1) && (col == -1))
		return;

	/* Translators: "Ln" is an abbreviation for "Line", Col is an abbreviation for "Column". Please,
	use abbreviations if possible to avoid space problems. */
	msg = g_strdup_printf (_("  Ln %d, Col %d"), line, col);

	gtk_statusbar_push (GTK_STATUSBAR (statusbar->priv->cursor_position_statusbar), 0, msg);

      	g_free (msg);
}

static gboolean
remove_message_timeout (GeditStatusbar *statusbar)
{
	gtk_statusbar_remove (GTK_STATUSBAR (statusbar),
			      statusbar->priv->flash_context_id,
			      statusbar->priv->flash_message_id);

	/* remove the timeout */
	statusbar->priv->flash_timeout = 0;
  	return FALSE;
}

/**
 * gedit_statusbar_flash_message:
 * @statusbar: a #GeditStatusbar
 * @context_id: message context_id
 * @format: message to flash on the statusbar
 *
 * Flash a temporary message on the statusbar.
 */
void
gedit_statusbar_flash_message (GeditStatusbar *statusbar,
			       guint           context_id,
			       const gchar    *format, ...)
{
	const guint32 flash_length = 3000; /* three seconds */
	va_list args;
	gchar *msg;

	g_return_if_fail (GEDIT_IS_STATUSBAR (statusbar));
	g_return_if_fail (format != NULL);

	va_start (args, format);
	msg = g_strdup_vprintf (format, args);
	va_end (args);

	/* remove a currently ongoing flash message */
	if (statusbar->priv->flash_timeout > 0)
	{
		g_source_remove (statusbar->priv->flash_timeout);
		statusbar->priv->flash_timeout = 0;

		gtk_statusbar_remove (GTK_STATUSBAR (statusbar),
				      statusbar->priv->flash_context_id,
				      statusbar->priv->flash_message_id);
	}

	statusbar->priv->flash_context_id = context_id;
	statusbar->priv->flash_message_id = gtk_statusbar_push (GTK_STATUSBAR (statusbar),
								context_id,
								msg);

	statusbar->priv->flash_timeout = g_timeout_add (flash_length,
							(GtkFunction) remove_message_timeout,
							statusbar);

	g_free (msg);
}

void
gedit_statusbar_set_window_state (GeditStatusbar   *statusbar,
				  GeditWindowState  state,
				  gint              num_of_errors)
{
	g_return_if_fail (GEDIT_IS_STATUSBAR (statusbar));

	gtk_widget_hide (statusbar->priv->state_frame);
	gtk_widget_hide (statusbar->priv->save_image);
	gtk_widget_hide (statusbar->priv->load_image);
	gtk_widget_hide (statusbar->priv->print_image);

	if (state & GEDIT_WINDOW_STATE_SAVING)
	{
		gtk_widget_show (statusbar->priv->state_frame);
		gtk_widget_show (statusbar->priv->save_image);
	}
	if (state & GEDIT_WINDOW_STATE_LOADING)
	{
		gtk_widget_show (statusbar->priv->state_frame);
		gtk_widget_show (statusbar->priv->load_image);
	}

	if (state & GEDIT_WINDOW_STATE_PRINTING)
	{
		gtk_widget_show (statusbar->priv->state_frame);
		gtk_widget_show (statusbar->priv->print_image);
	}

	if (state & GEDIT_WINDOW_STATE_ERROR)
	{
	 	gchar *tip;

 		tip = g_strdup_printf (ngettext("There is a tab with errors",
						"There are %d tabs with errors",
						num_of_errors),
			       		num_of_errors);

		gtk_widget_set_tooltip_text (statusbar->priv->error_event_box,
					     tip);
		g_free (tip);

		gtk_widget_show (statusbar->priv->error_frame);
	}
	else
	{
		gtk_widget_hide (statusbar->priv->error_frame);
	}
}


