/*
 * xed-view-frame.c
 * This file is part of xed
 *
 * Copyright (C) 2010 - Ignacio Casal Quinteiro
 *
 * xed is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * xed is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xed; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "xed-view-frame.h"
#include "xed-marshal.h"
#include "xed-debug.h"
#include "xed-utils.h"

#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>
#include <stdlib.h>

#define XED_VIEW_FRAME_SEARCH_DIALOG_TIMEOUT (30*1000) /* 30 seconds */

#define SEARCH_POPUP_MARGIN 12

#define XED_VIEW_FRAME_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), XED_TYPE_VIEW_FRAME, XedViewFramePrivate))

struct _XedViewFramePrivate
{
    GtkWidget *view;
    GtkWidget *overlay;

    GtkTextMark *start_mark;

    GtkWidget *revealer;
    GtkWidget *search_entry;

    guint flush_timeout_id;
    glong search_entry_focus_out_id;
    glong search_entry_changed_id;
    gboolean disable_popdown;
};

enum
{
    PROP_0,
    PROP_DOCUMENT,
    PROP_VIEW
};

typedef enum
{
    SEARCH_STATE_NORMAL,
    SEARCH_STATE_NOT_FOUND
} SearchState;

G_DEFINE_TYPE (XedViewFrame, xed_view_frame, GTK_TYPE_BOX)

static void
xed_view_frame_finalize (GObject *object)
{
    G_OBJECT_CLASS (xed_view_frame_parent_class)->finalize (object);
}

static void
xed_view_frame_dispose (GObject *object)
{
    XedViewFrame *frame = XED_VIEW_FRAME (object);
    GtkTextBuffer *buffer = NULL;

    if (frame->priv->view != NULL)
    {
        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (frame->priv->view));
    }

    if (frame->priv->flush_timeout_id != 0)
    {
        g_source_remove (frame->priv->flush_timeout_id);
        frame->priv->flush_timeout_id = 0;
    }

    if (buffer != NULL)
    {
        GtkSourceFile *file = xed_document_get_file (XED_DOCUMENT (buffer));
        gtk_source_file_set_mount_operation_factory (file, NULL, NULL, NULL);
    }

    G_OBJECT_CLASS (xed_view_frame_parent_class)->dispose (object);
}

static void
xed_view_frame_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
    XedViewFrame *frame = XED_VIEW_FRAME (object);

    switch (prop_id)
    {
        case PROP_DOCUMENT:
            g_value_set_object (value, xed_view_frame_get_document (frame));
            break;
        case PROP_VIEW:
            g_value_set_object (value, xed_view_frame_get_view (frame));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
hide_search_widget (XedViewFrame *frame,
                    gboolean      cancel)
{
    GtkTextBuffer *buffer;

    g_signal_handler_block (frame->priv->search_entry, frame->priv->search_entry_focus_out_id);

    if (frame->priv->flush_timeout_id != 0)
    {
        g_source_remove (frame->priv->flush_timeout_id);
        frame->priv->flush_timeout_id = 0;
    }

    gtk_revealer_set_reveal_child (GTK_REVEALER (frame->priv->revealer), FALSE);

    if (cancel)
    {
        GtkTextBuffer *buffer;
        GtkTextIter iter;

        buffer = GTK_TEXT_BUFFER (gtk_text_view_get_buffer (GTK_TEXT_VIEW (frame->priv->view)));
        gtk_text_buffer_get_iter_at_mark (buffer, &iter, frame->priv->start_mark);
        gtk_text_buffer_place_cursor (buffer, &iter);

        xed_view_scroll_to_cursor (XED_VIEW (frame->priv->view));
    }

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (frame->priv->view));
    gtk_text_buffer_delete_mark (buffer, frame->priv->start_mark);

    /* Make sure the view is the one who has the focus when we destroy
       the search widget */
    gtk_widget_grab_focus (frame->priv->view);

    g_signal_handler_unblock (frame->priv->search_entry, frame->priv->search_entry_focus_out_id);
}

static gboolean
search_entry_flush_timeout (XedViewFrame *frame)
{
    frame->priv->flush_timeout_id = 0;
    hide_search_widget (frame, FALSE);

    return FALSE;
}

static void
set_search_state (XedViewFrame *frame,
                  SearchState   state)
{
    GtkStyleContext *context;

    context = gtk_widget_get_style_context (GTK_WIDGET (frame->priv->search_entry));

    if (state == SEARCH_STATE_NOT_FOUND)
    {
        gtk_style_context_add_class (context, GTK_STYLE_CLASS_ERROR);
    }
    else
    {
        gtk_style_context_remove_class (context, GTK_STYLE_CLASS_ERROR);
    }
}

static gboolean
search_widget_key_press_event (GtkWidget    *widget,
                               GdkEventKey  *event,
                               XedViewFrame *frame)
{
    if (event->keyval == GDK_KEY_Escape)
    {

        hide_search_widget (frame, TRUE);
        return GDK_EVENT_STOP;
    }

    return GDK_EVENT_PROPAGATE;
}

static void
search_entry_activate (GtkEntry     *entry,
                       XedViewFrame *frame)
{
    hide_search_widget (frame, FALSE);
}

static void
search_entry_insert_text (GtkEditable  *editable,
                          const gchar  *text,
                          gint          length,
                          gint         *position,
                          XedViewFrame *frame)
{
    gunichar c;
    const gchar *p;
    const gchar *end;
    const gchar *next;

    p = text;
    end = text + length;

    if (p == end)
        return;

    c = g_utf8_get_char (p);

    if (((c == '-' || c == '+') && *position == 0) ||
        (c == ':' && *position != 0))
    {
        gchar *s = NULL;

        if (c == ':')
        {
            s = gtk_editable_get_chars (editable, 0, -1);
            s = g_utf8_strchr (s, -1, ':');
        }

        if (s == NULL || s == p)
        {
            next = g_utf8_next_char (p);
            p = next;
        }

        g_free (s);
    }

    while (p != end)
    {
        next = g_utf8_next_char (p);

        c = g_utf8_get_char (p);

        if (!g_unichar_isdigit (c))
        {
            g_signal_stop_emission_by_name (editable, "insert_text");
            gtk_widget_error_bell (frame->priv->search_entry);
            break;
        }

        p = next;
    }
}

static void
search_init (GtkWidget      *entry,
             XedViewFrame *frame)
{
    const gchar *entry_text;

    /* renew the flush timeout */
    if (frame->priv->flush_timeout_id != 0)
    {
        g_source_remove (frame->priv->flush_timeout_id);
        frame->priv->flush_timeout_id = g_timeout_add (XED_VIEW_FRAME_SEARCH_DIALOG_TIMEOUT,
                                                               (GSourceFunc)search_entry_flush_timeout, frame);
    }

    entry_text = gtk_entry_get_text (GTK_ENTRY (entry));

    if (*entry_text != '\0')
    {
        gboolean moved, moved_offset;
        gint line;
        gint offset_line = 0;
        gint line_offset = 0;
        gchar **split_text = NULL;
        const gchar *text;
        GtkTextIter iter;
        XedDocument *doc;

        doc = xed_view_frame_get_document (frame);
        gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (doc), &iter, frame->priv->start_mark);
        split_text = g_strsplit (entry_text, ":", -1);

        if (g_strv_length (split_text) > 1)
        {
            text = split_text[0];
        }
        else
        {
            text = entry_text;
        }

        if (*text == '-')
        {
            gint cur_line = gtk_text_iter_get_line (&iter);

            if (*(text + 1) != '\0')
            {
                offset_line = MAX (atoi (text + 1), 0);
            }

            line = MAX (cur_line - offset_line, 0);
        }
        else if (*entry_text == '+')
        {
            gint cur_line = gtk_text_iter_get_line (&iter);

            if (*(text + 1) != '\0')
            {
                offset_line = MAX (atoi (text + 1), 0);
            }

            line = cur_line + offset_line;
        }
        else
        {
            line = MAX (atoi (text) - 1, 0);
        }

        if (split_text[1] != NULL)
        {
            line_offset = atoi (split_text[1]);
        }

        g_strfreev (split_text);

        moved = xed_document_goto_line (doc, line);
        moved_offset = xed_document_goto_line_offset (doc, line, line_offset);

        xed_view_scroll_to_cursor (XED_VIEW (frame->priv->view));

        if (!moved || !moved_offset)
        {
            set_search_state (frame, SEARCH_STATE_NOT_FOUND);
        }
        else
        {
            set_search_state (frame, SEARCH_STATE_NORMAL);
        }
    }
}

static gboolean
search_entry_focus_out_event (GtkWidget     *widget,
                              GdkEventFocus *event,
                              XedViewFrame  *frame)
{
    if (frame->priv->disable_popdown)
    {
        return GDK_EVENT_STOP;
    }

    hide_search_widget (frame, FALSE);
    return GDK_EVENT_PROPAGATE;
}

static void
search_enable_popdown (GtkWidget    *widget,
                       XedViewFrame *frame)
{
    frame->priv->disable_popdown = FALSE;
}

static void
search_entry_populate_popup (GtkEntry     *entry,
                             GtkMenu      *menu,
                             XedViewFrame *frame)
{
    frame->priv->disable_popdown = TRUE;
    g_signal_connect (menu, "hide",
                      G_CALLBACK (search_enable_popdown), frame);
}

static GtkWidget *
create_search_widget (XedViewFrame *frame)
{
    GtkWidget *search_widget;
    GtkWidget *hbox;
    GtkStyleContext *context;

    /* wrap it in a frame, so we can specify borders etc */
    search_widget = gtk_frame_new (NULL);
    context = gtk_widget_get_style_context (search_widget);
    gtk_style_context_add_class (context, "xed-goto-line-box");
    gtk_widget_show (search_widget);

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add (GTK_CONTAINER (search_widget), hbox);
    gtk_widget_show (hbox);

    g_signal_connect (hbox, "key-press-event",
                      G_CALLBACK (search_widget_key_press_event), frame);

    /* add entry */
    frame->priv->search_entry = gtk_entry_new ();
    gtk_widget_set_tooltip_text (frame->priv->search_entry, _("Line you want to move the cursor to"));
    gtk_entry_set_icon_from_icon_name (GTK_ENTRY (frame->priv->search_entry),
                                       GTK_ENTRY_ICON_PRIMARY, "go-jump-symbolic");
    gtk_widget_show (frame->priv->search_entry);

    g_signal_connect (frame->priv->search_entry, "activate",
                      G_CALLBACK (search_entry_activate), frame);
    g_signal_connect (frame->priv->search_entry, "insert_text",
                      G_CALLBACK (search_entry_insert_text), frame);
    g_signal_connect (frame->priv->search_entry, "populate-popup",
                      G_CALLBACK (search_entry_populate_popup), frame);
    frame->priv->search_entry_changed_id = g_signal_connect (frame->priv->search_entry, "changed",
                                                             G_CALLBACK (search_init), frame);
    frame->priv->search_entry_focus_out_id = g_signal_connect (frame->priv->search_entry, "focus-out-event",
                                                               G_CALLBACK (search_entry_focus_out_event), frame);

    gtk_container_add (GTK_CONTAINER (hbox), frame->priv->search_entry);

    return search_widget;
}

static void
init_search_entry (XedViewFrame *frame)
{
    GtkTextBuffer *buffer;
    gint line;
    gchar *line_str;
    GtkTextIter iter;

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (frame->priv->view));
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, frame->priv->start_mark);
    line = gtk_text_iter_get_line (&iter);
    line_str = g_strdup_printf ("%d", line + 1);

    gtk_entry_set_text (GTK_ENTRY (frame->priv->search_entry), line_str);
    gtk_editable_select_region (GTK_EDITABLE (frame->priv->search_entry), 0, -1);

    g_free (line_str);

    return;
}

static void
start_interactive_search_real (XedViewFrame *frame)
{
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    GtkTextMark *mark;

    if (gtk_revealer_get_reveal_child (GTK_REVEALER (frame->priv->revealer)))
    {
        gtk_editable_select_region (GTK_EDITABLE (frame->priv->search_entry), 0, -1);
        return;
    }

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (frame->priv->view));

    mark = gtk_text_buffer_get_insert (buffer);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);

    frame->priv->start_mark = gtk_text_buffer_create_mark (buffer, NULL, &iter, FALSE);

    gtk_revealer_set_reveal_child (GTK_REVEALER (frame->priv->revealer), TRUE);

    /* NOTE: we must be very careful here to not have any text before
       focusing the entry because when the entry is focused the text is
       selected, and gtk+ doesn't allow us to have more than one selection
       active */
    g_signal_handler_block (frame->priv->search_entry, frame->priv->search_entry_changed_id);
    gtk_entry_set_text (GTK_ENTRY (frame->priv->search_entry), "");
    g_signal_handler_unblock (frame->priv->search_entry, frame->priv->search_entry_changed_id);

    /* We need to grab the focus after the widget has been added */
    gtk_widget_grab_focus (frame->priv->search_entry);

    init_search_entry (frame);

    frame->priv->flush_timeout_id = g_timeout_add (XED_VIEW_FRAME_SEARCH_DIALOG_TIMEOUT,
                                                           (GSourceFunc) search_entry_flush_timeout, frame);
}

static void
xed_view_frame_class_init (XedViewFrameClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = xed_view_frame_finalize;
    object_class->dispose = xed_view_frame_dispose;
    object_class->get_property = xed_view_frame_get_property;

    g_object_class_install_property (object_class, PROP_DOCUMENT,
                                     g_param_spec_object ("document",
                                                          "Document",
                                                          "The Document",
                                                          XED_TYPE_DOCUMENT,
                                                          G_PARAM_READABLE |
                                                          G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (object_class, PROP_VIEW,
                                     g_param_spec_object ("view",
                                                          "View",
                                                          "The View",
                                                          XED_TYPE_VIEW,
                                                          G_PARAM_READABLE |
                                                          G_PARAM_STATIC_STRINGS));

    g_type_class_add_private (object_class, sizeof (XedViewFramePrivate));
}

static GMountOperation *
view_frame_mount_operation_factory (GtkSourceFile *file,
                                    gpointer       user_data)
{
    GtkWidget *view_frame = user_data;
    GtkWidget *window = gtk_widget_get_toplevel (view_frame);

    return gtk_mount_operation_new (GTK_WINDOW (window));
}

static void
xed_view_frame_init (XedViewFrame *frame)
{
    XedDocument *doc;
    GtkSourceFile *file;
    GtkWidget *sw;
    GdkRGBA transparent = {0, 0, 0, 0};

    frame->priv = XED_VIEW_FRAME_GET_PRIVATE (frame);

    gtk_orientable_set_orientation (GTK_ORIENTABLE (frame), GTK_ORIENTATION_VERTICAL);

    doc = xed_document_new ();
    file = xed_document_get_file (doc);

    gtk_source_file_set_mount_operation_factory (file, view_frame_mount_operation_factory, frame, NULL);

    frame->priv->view = xed_view_new (doc);
    gtk_widget_set_vexpand (frame->priv->view, TRUE);
    gtk_widget_show (frame->priv->view);

    g_object_unref (doc);

    /* Create the scrolled window */
    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_container_add (GTK_CONTAINER (sw), frame->priv->view);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_show (sw);

    frame->priv->overlay = gtk_overlay_new ();
    gtk_container_add (GTK_CONTAINER (frame->priv->overlay), sw);
    gtk_widget_override_background_color (frame->priv->overlay, 0, &transparent);
    gtk_widget_show (frame->priv->overlay);

    gtk_box_pack_start (GTK_BOX (frame), frame->priv->overlay, TRUE, TRUE, 0);

    /* Add revealer */
    frame->priv->revealer = gtk_revealer_new ();
    gtk_container_add (GTK_CONTAINER (frame->priv->revealer), create_search_widget (frame));
    gtk_widget_show (frame->priv->revealer);
    gtk_widget_set_halign (frame->priv->revealer, GTK_ALIGN_END);
    gtk_widget_set_valign (frame->priv->revealer, GTK_ALIGN_START);

    if (gtk_widget_get_direction (frame->priv->revealer) == GTK_TEXT_DIR_LTR)
    {
        gtk_widget_set_margin_right (frame->priv->revealer, SEARCH_POPUP_MARGIN);
    }
    else
    {
        gtk_widget_set_margin_left (frame->priv->revealer, SEARCH_POPUP_MARGIN);
    }

    gtk_overlay_add_overlay (GTK_OVERLAY (frame->priv->overlay), frame->priv->revealer);
}

XedViewFrame *
xed_view_frame_new ()
{
    return g_object_new (XED_TYPE_VIEW_FRAME, NULL);
}

XedDocument *
xed_view_frame_get_document (XedViewFrame *frame)
{
    g_return_val_if_fail (XED_IS_VIEW_FRAME (frame), NULL);

    return XED_DOCUMENT (gtk_text_view_get_buffer (GTK_TEXT_VIEW (frame->priv->view)));
}

XedView *
xed_view_frame_get_view (XedViewFrame *frame)
{
    g_return_val_if_fail (XED_IS_VIEW_FRAME (frame), NULL);

    return XED_VIEW (frame->priv->view);
}

void
xed_view_frame_popup_goto_line (XedViewFrame *frame)
{
    g_return_if_fail (XED_IS_VIEW_FRAME (frame));

    start_interactive_search_real (frame);
}

gboolean
xed_view_frame_get_search_popup_visible (XedViewFrame *frame)
{
    g_return_val_if_fail (XED_IS_VIEW_FRAME (frame), FALSE);

    return gtk_revealer_get_child_revealed (GTK_REVEALER (frame->priv->revealer));
}
