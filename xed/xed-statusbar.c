/*
 * xed-statusbar.c
 * This file is part of xed
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

/*
 * Modified by the xed Team, 2005. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#include <config.h>
#include <string.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "xed-statusbar.h"

#define XED_STATUSBAR_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
                                          XED_TYPE_STATUSBAR,\
                                          XedStatusbarPrivate))

struct _XedStatusbarPrivate
{
    GtkWidget *overwrite_mode_label;
    GtkWidget *cursor_position_label;

    GtkWidget *state_frame;
    GtkWidget *load_image;
    GtkWidget *save_image;
    GtkWidget *print_image;

    GtkWidget *error_frame;
    GtkWidget *error_event_box;

    /* tmp flash timeout data */
    guint flash_timeout;
    guint flash_context_id;
    guint flash_message_id;
};

G_DEFINE_TYPE(XedStatusbar, xed_statusbar, GTK_TYPE_STATUSBAR)


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
xed_statusbar_dispose (GObject *object)
{
    XedStatusbar *statusbar = XED_STATUSBAR (object);

    if (statusbar->priv->flash_timeout > 0)
    {
        g_source_remove (statusbar->priv->flash_timeout);
        statusbar->priv->flash_timeout = 0;
    }

    G_OBJECT_CLASS (xed_statusbar_parent_class)->dispose (object);
}

static void
xed_statusbar_class_init (XedStatusbarClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = xed_statusbar_dispose;

    g_type_class_add_private (object_class, sizeof (XedStatusbarPrivate));
}

#define CURSOR_POSITION_LABEL_WIDTH_CHARS 18

static void
xed_statusbar_init (XedStatusbar *statusbar)
{
    GtkWidget *hbox;
    GtkWidget *error_image;

    statusbar->priv = XED_STATUSBAR_GET_PRIVATE (statusbar);

    gtk_widget_set_margin_top (GTK_WIDGET (statusbar), 0);
    gtk_widget_set_margin_bottom (GTK_WIDGET (statusbar), 0);

    statusbar->priv->overwrite_mode_label = gtk_label_new (NULL);
    gtk_label_set_width_chars (GTK_LABEL (statusbar->priv->overwrite_mode_label), get_overwrite_mode_length ());
    gtk_widget_show (statusbar->priv->overwrite_mode_label);
    gtk_box_pack_end (GTK_BOX (statusbar), statusbar->priv->overwrite_mode_label, FALSE, TRUE, 0);
    gtk_widget_set_margin_end (GTK_WIDGET (statusbar->priv->overwrite_mode_label), 6);

    statusbar->priv->cursor_position_label = gtk_label_new (NULL);
    gtk_label_set_width_chars (GTK_LABEL (statusbar->priv->cursor_position_label), CURSOR_POSITION_LABEL_WIDTH_CHARS);
    gtk_widget_show (statusbar->priv->cursor_position_label);
    gtk_box_pack_end (GTK_BOX (statusbar), statusbar->priv->cursor_position_label, FALSE, TRUE, 0);

    statusbar->priv->state_frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (statusbar->priv->state_frame), GTK_SHADOW_IN);

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add (GTK_CONTAINER (statusbar->priv->state_frame), hbox);

    statusbar->priv->load_image = gtk_image_new_from_icon_name ("document-open-symbolic", GTK_ICON_SIZE_MENU);
    statusbar->priv->save_image = gtk_image_new_from_icon_name ("document-save-symbolic", GTK_ICON_SIZE_MENU);
    statusbar->priv->print_image = gtk_image_new_from_icon_name ("printer-symbolic", GTK_ICON_SIZE_MENU);

    gtk_widget_show (hbox);

    gtk_box_pack_start (GTK_BOX (hbox), statusbar->priv->load_image, FALSE, TRUE, 4);
    gtk_box_pack_start (GTK_BOX (hbox), statusbar->priv->save_image, FALSE, TRUE, 4);
    gtk_box_pack_start (GTK_BOX (hbox), statusbar->priv->print_image, FALSE, TRUE, 4);

    gtk_box_pack_start (GTK_BOX (statusbar), statusbar->priv->state_frame, FALSE, TRUE, 0);

    statusbar->priv->error_frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (statusbar->priv->error_frame), GTK_SHADOW_IN);

    error_image = gtk_image_new_from_icon_name ("dialog-error", GTK_ICON_SIZE_MENU);

    gtk_widget_set_margin_start (error_image, 4);
    gtk_widget_set_margin_end (error_image, 4);
    gtk_widget_set_margin_top (error_image, 0);
    gtk_widget_set_margin_bottom (error_image, 0);

    statusbar->priv->error_event_box = gtk_event_box_new ();
    gtk_event_box_set_visible_window  (GTK_EVENT_BOX (statusbar->priv->error_event_box), FALSE);
    gtk_widget_show (statusbar->priv->error_event_box);

    gtk_container_add (GTK_CONTAINER (statusbar->priv->error_frame), statusbar->priv->error_event_box);
    gtk_container_add (GTK_CONTAINER (statusbar->priv->error_event_box), error_image);

    gtk_box_pack_start (GTK_BOX (statusbar), statusbar->priv->error_frame, FALSE, TRUE, 0);

    gtk_style_context_add_class (gtk_widget_get_style_context GTK_WIDGET (statusbar), "xed-statusbar");
}

/**
 * xed_statusbar_new:
 *
 * Creates a new #XedStatusbar.
 *
 * Return value: the new #XedStatusbar object
 **/
GtkWidget *
xed_statusbar_new (void)
{
    return GTK_WIDGET (g_object_new (XED_TYPE_STATUSBAR, NULL));
}

/**
 * xed_statusbar_set_overwrite:
 * @statusbar: a #XedStatusbar
 * @overwrite: if the overwrite mode is set
 *
 * Sets the overwrite mode on the statusbar.
 **/
void
xed_statusbar_set_overwrite (XedStatusbar *statusbar,
                             gboolean      overwrite)
{
    gchar *msg;

    g_return_if_fail (XED_IS_STATUSBAR (statusbar));

    msg = get_overwrite_mode_string (overwrite);

    gtk_label_set_text (GTK_LABEL (statusbar->priv->overwrite_mode_label), msg);

    g_free (msg);
}

void
xed_statusbar_clear_overwrite (XedStatusbar *statusbar)
{
    g_return_if_fail (XED_IS_STATUSBAR (statusbar));

    gtk_label_set_text (GTK_LABEL (statusbar->priv->overwrite_mode_label), NULL);
}

/**
 * xed_statusbar_cursor_position:
 * @statusbar: an #XedStatusbar
 * @line: line position
 * @col: column position
 *
 * Sets the cursor position on the statusbar.
 **/
void
xed_statusbar_set_cursor_position (XedStatusbar *statusbar,
                                   gint          line,
                                   gint          col)
{
    gchar *msg = NULL;

    g_return_if_fail (XED_IS_STATUSBAR (statusbar));

    if ((line >= 0) || (col >= 0))
    {
        /* Translators: "Ln" is an abbreviation for "Line", Col is an abbreviation for "Column". Please,
        use abbreviations if possible to avoid space problems. */
        msg = g_strdup_printf (_("  Ln %d, Col %d"), line, col);
    }

    gtk_label_set_text (GTK_LABEL (statusbar->priv->cursor_position_label), msg);

    g_free (msg);
}

static gboolean
remove_message_timeout (XedStatusbar *statusbar)
{
    gtk_statusbar_remove (GTK_STATUSBAR (statusbar),
                          statusbar->priv->flash_context_id,
                          statusbar->priv->flash_message_id);

    /* remove the timeout */
    statusbar->priv->flash_timeout = 0;
    return FALSE;
}

/* FIXME this is an issue for introspection */
/**
 * xed_statusbar_flash_message:
 * @statusbar: a #XedStatusbar
 * @context_id: message context_id
 * @format: message to flash on the statusbar
 *
 * Flash a temporary message on the statusbar.
 */
void
xed_statusbar_flash_message (XedStatusbar *statusbar,
                             guint         context_id,
                             const gchar  *format, ...)
{
    const guint32 flash_length = 3000; /* three seconds */
    va_list args;
    gchar *msg;

    g_return_if_fail (XED_IS_STATUSBAR (statusbar));
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
    statusbar->priv->flash_message_id = gtk_statusbar_push (GTK_STATUSBAR (statusbar), context_id, msg);

    statusbar->priv->flash_timeout = g_timeout_add (flash_length,
                                                    (GSourceFunc) remove_message_timeout,
                                                    statusbar);

    g_free (msg);
}

void
xed_statusbar_set_window_state (XedStatusbar   *statusbar,
                                XedWindowState  state,
                                gint            num_of_errors)
{
    g_return_if_fail (XED_IS_STATUSBAR (statusbar));

    gtk_widget_hide (statusbar->priv->state_frame);
    gtk_widget_hide (statusbar->priv->save_image);
    gtk_widget_hide (statusbar->priv->load_image);
    gtk_widget_hide (statusbar->priv->print_image);

    if (state & XED_WINDOW_STATE_SAVING)
    {
        gtk_widget_show (statusbar->priv->state_frame);
        gtk_widget_show (statusbar->priv->save_image);
    }
    if (state & XED_WINDOW_STATE_LOADING)
    {
        gtk_widget_show (statusbar->priv->state_frame);
        gtk_widget_show (statusbar->priv->load_image);
    }

    if (state & XED_WINDOW_STATE_PRINTING)
    {
        gtk_widget_show (statusbar->priv->state_frame);
        gtk_widget_show (statusbar->priv->print_image);
    }

    if (state & XED_WINDOW_STATE_ERROR)
    {
        gchar *tip;

        tip = g_strdup_printf (ngettext("There is a tab with errors",
                               "There are %d tabs with errors",
                               num_of_errors),
                               num_of_errors);

        gtk_widget_set_tooltip_text (statusbar->priv->error_event_box, tip);
        g_free (tip);

        gtk_widget_show (statusbar->priv->error_frame);
    }
    else
    {
        gtk_widget_hide (statusbar->priv->error_frame);
    }
}


