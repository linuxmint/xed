/*
 * xed-tab.c
 * This file is part of xed
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
 * Modified by the xed Team, 2005. See the AUTHORS file for a
 * list of people on the xed Team.
 * See the ChangeLog files for a list of changes.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <glib/gi18n.h>
#include <gio/gio.h>

#include "xed-app.h"
#include "xed-notebook.h"
#include "xed-tab.h"
#include "xed-utils.h"
#include "xed-io-error-info-bar.h"
#include "xed-print-job.h"
#include "xed-print-preview.h"
#include "xed-progress-info-bar.h"
#include "xed-debug.h"
#include "xed-document.h"
#include "xed-document-private.h"
#include "xed-enum-types.h"
#include "xed-settings.h"
#include "xed-view-frame.h"

#define XED_TAB_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), XED_TYPE_TAB, XedTabPrivate))

#define XED_TAB_KEY "XED_TAB_KEY"

struct _XedTabPrivate
{
    GSettings *editor;

    XedTabState state;

    XedViewFrame *frame;

    GtkWidget *info_bar;
    GtkWidget *print_preview;

    XedPrintJob *print_job;

    GTask *task_saver;
    GtkSourceFileSaverFlags save_flags;

    /* tmp data for loading */
    GtkSourceFileLoader *loader;
    GCancellable *cancellable;
    gint tmp_line_pos;
    guint idle_scroll;

    GTimer *timer;
    guint times_called;

    guint auto_save_interval;
    guint auto_save_timeout;

    gint editable : 1;
    gint auto_save : 1;

    gint ask_if_externally_modified : 1;

    /*tmp data for loading */
    guint user_requested_encoding : 1;
};

typedef struct _SaverData SaverData;

struct _SaverData
{
    GtkSourceFileSaver *saver;

    /* Notes about the create_backup saver flag:
     * - At the beginning of a new file saving, force_no_backup is FALSE.
     *   The create_backup flag is set to the saver if it is enabled in
     *   GSettings and if it isn't an auto-save.
     * - If creating the backup gives an error, and if the user wants to
     *   save the file without the backup, force_no_backup is set to TRUE
     *   and the create_backup flag is removed from the saver.
     *   force_no_backup as TRUE means that the create_backup flag should
     *   never be added again to the saver (for the current file saving).
     * - When another error occurs and if the user explicitly retry again
     *   the file saving, the create_backup flag is added to the saver if
     *   (1) it is enabled in GSettings, (2) if force_no_backup is FALSE.
     * - The create_backup flag is added when the user expressed his or her
     *   willing to save the file, by pressing a button for example. For an
     *   auto-save, the create_backup flag is thus not added initially, but
     *   can be added later when an error occurs and the user clicks on a
     *   button in the info bar to retry the file saving.
     */
    guint force_no_backup : 1;
};

G_DEFINE_TYPE(XedTab, xed_tab, GTK_TYPE_BOX)

enum
{
    PROP_0,
    PROP_NAME,
    PROP_STATE,
    PROP_AUTO_SAVE,
    PROP_AUTO_SAVE_INTERVAL,
    PROP_CAN_CLOSE
};

static gboolean xed_tab_auto_save (XedTab *tab);

static void load (XedTab                  *tab,
                  const GtkSourceEncoding *encoding,
                  gint                     line_pos);

static void save (XedTab *tab);

static SaverData *
saver_data_new (void)
{
    return g_slice_new0 (SaverData);
}

static void
saver_data_free (SaverData *data)
{
    if (data != NULL)
    {
        if (data->saver != NULL)
        {
            g_object_unref (data->saver);
        }

        g_slice_free (SaverData, data);
    }
}

static void
install_auto_save_timeout (XedTab *tab)
{
    if (tab->priv->auto_save_timeout == 0)
    {
        g_return_if_fail (tab->priv->auto_save_interval > 0);

        tab->priv->auto_save_timeout = g_timeout_add_seconds (tab->priv->auto_save_interval * 60,
                                                              (GSourceFunc) xed_tab_auto_save,
                                                              tab);
    }
}

static void
remove_auto_save_timeout (XedTab *tab)
{
    xed_debug (DEBUG_TAB);

    if (tab->priv->auto_save_timeout > 0)
    {
        g_source_remove (tab->priv->auto_save_timeout);
        tab->priv->auto_save_timeout = 0;
    }
}

static void
update_auto_save_timeout (XedTab *tab)
{
    gboolean good_state;
    XedDocument *doc;

    xed_debug (DEBUG_TAB);

    good_state = (tab->priv->state == XED_TAB_STATE_NORMAL ||
                  tab->priv->state == XED_TAB_STATE_SHOWING_PRINT_PREVIEW);

    doc = xed_tab_get_document (tab);

    if (good_state &&
       tab->priv->auto_save &&
       !xed_document_is_untitled (doc) &&
       !xed_document_get_readonly (doc))
    {
        install_auto_save_timeout (tab);
    }
    else
    {
        remove_auto_save_timeout (tab);
    }
}

static void
xed_tab_get_property (GObject    *object,
                      guint       prop_id,
                      GValue     *value,
                      GParamSpec *pspec)
{
    XedTab *tab = XED_TAB (object);

    switch (prop_id)
    {
        case PROP_NAME:
            g_value_take_string (value, _xed_tab_get_name (tab));
            break;
        case PROP_STATE:
            g_value_set_enum (value, xed_tab_get_state (tab));
            break;
        case PROP_AUTO_SAVE:
            g_value_set_boolean (value, xed_tab_get_auto_save_enabled (tab));
            break;
        case PROP_AUTO_SAVE_INTERVAL:
            g_value_set_int (value, xed_tab_get_auto_save_interval (tab));
            break;
        case PROP_CAN_CLOSE:
            g_value_set_boolean (value, _xed_tab_get_can_close (tab));
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
xed_tab_set_property (GObject      *object,
                      guint         prop_id,
                      const GValue *value,
                      GParamSpec   *pspec)
{
    XedTab *tab = XED_TAB (object);

    switch (prop_id)
    {
        case PROP_AUTO_SAVE:
            xed_tab_set_auto_save_enabled (tab, g_value_get_boolean (value));
            break;
        case PROP_AUTO_SAVE_INTERVAL:
            xed_tab_set_auto_save_interval (tab, g_value_get_int (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
clear_loading (XedTab *tab)
{
    g_clear_object (&tab->priv->loader);
    g_clear_object (&tab->priv->cancellable);
}

static void
xed_tab_dispose (GObject *object)
{
    XedTab *tab = XED_TAB (object);

    g_clear_object (&tab->priv->editor);
    g_clear_object (&tab->priv->task_saver);

    clear_loading (tab);

    G_OBJECT_CLASS (xed_tab_parent_class)->dispose (object);
}

static void
xed_tab_finalize (GObject *object)
{
    XedTab *tab = XED_TAB (object);

    if (tab->priv->timer != NULL)
    {
        g_timer_destroy (tab->priv->timer);
    }

    remove_auto_save_timeout (tab);

    if (tab->priv->idle_scroll != 0)
    {
        g_source_remove (tab->priv->idle_scroll);
        tab->priv->idle_scroll = 0;
    }

    G_OBJECT_CLASS (xed_tab_parent_class)->finalize (object);
}

static void
xed_tab_class_init (XedTabClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = xed_tab_dispose;
    object_class->finalize = xed_tab_finalize;
    object_class->get_property = xed_tab_get_property;
    object_class->set_property = xed_tab_set_property;

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
                                                        XED_TYPE_TAB_STATE,
                                                        XED_TAB_STATE_NORMAL,
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

    g_object_class_install_property (object_class,
                                     PROP_CAN_CLOSE,
                                     g_param_spec_boolean ("can-close",
                                                           "Can close",
                                                           "Wheather the tab can be closed",
                                                           TRUE,
                                                           G_PARAM_READABLE |
                                                           G_PARAM_STATIC_STRINGS));

    g_type_class_add_private (object_class, sizeof (XedTabPrivate));
}

/**
 * xed_tab_get_state:
 * @tab: a #XedTab
 *
 * Gets the #XedTabState of @tab.
 *
 * Returns: the #XedTabState of @tab
 */
XedTabState
xed_tab_get_state (XedTab *tab)
{
    g_return_val_if_fail (XED_IS_TAB (tab), XED_TAB_STATE_NORMAL);

    return tab->priv->state;
}

static void
set_cursor_according_to_state (GtkTextView *view,
                               XedTabState  state)
{
    GdkCursor *cursor;
    GdkWindow *text_window;
    GdkWindow *left_window;

    text_window = gtk_text_view_get_window (view, GTK_TEXT_WINDOW_TEXT);
    left_window = gtk_text_view_get_window (view, GTK_TEXT_WINDOW_LEFT);

    if ((state == XED_TAB_STATE_LOADING)          ||
        (state == XED_TAB_STATE_REVERTING)        ||
        (state == XED_TAB_STATE_SAVING)           ||
        (state == XED_TAB_STATE_PRINTING)         ||
        (state == XED_TAB_STATE_PRINT_PREVIEWING) ||
        (state == XED_TAB_STATE_CLOSING))
    {
        cursor = gdk_cursor_new_for_display (gtk_widget_get_display (GTK_WIDGET (view)), GDK_WATCH);

        if (text_window != NULL)
        {
            gdk_window_set_cursor (text_window, cursor);
        }
        if (left_window != NULL)
        {
            gdk_window_set_cursor (left_window, cursor);
        }

        g_object_unref (cursor);
    }
    else
    {
        cursor = gdk_cursor_new_for_display (gtk_widget_get_display (GTK_WIDGET (view)), GDK_XTERM);

        if (text_window != NULL)
        {
            gdk_window_set_cursor (text_window, cursor);
        }
        if (left_window != NULL)
        {
            gdk_window_set_cursor (left_window, NULL);
        }

        g_object_unref (cursor);
    }
}

static void
view_realized (GtkTextView *view,
               XedTab      *tab)
{
    set_cursor_according_to_state (view, tab->priv->state);
}

static void
set_view_properties_according_to_state (XedTab      *tab,
                                        XedTabState  state)
{
    XedView *view;
    gboolean val;
    gboolean hl_current_line;

    hl_current_line = g_settings_get_boolean (tab->priv->editor, XED_SETTINGS_HIGHLIGHT_CURRENT_LINE);
    view = xed_view_frame_get_view (tab->priv->frame);
    val = ((state == XED_TAB_STATE_NORMAL) && (tab->priv->print_preview == NULL) && tab->priv->editable);
    gtk_text_view_set_editable (GTK_TEXT_VIEW (view), val);

    val = ((state != XED_TAB_STATE_LOADING) && (state != XED_TAB_STATE_CLOSING));
    gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (view), val);

    val = ((state != XED_TAB_STATE_LOADING) &&
           (state != XED_TAB_STATE_CLOSING) &&
           (hl_current_line));
    gtk_source_view_set_highlight_current_line (GTK_SOURCE_VIEW (view), val);
}

static void
xed_tab_set_state (XedTab      *tab,
                   XedTabState  state)
{
    g_return_if_fail (XED_IS_TAB (tab));
    g_return_if_fail ((state >= 0) && (state < XED_TAB_NUM_OF_STATES));

    if (tab->priv->state == state)
    {
        return;
    }

    tab->priv->state = state;

    set_view_properties_according_to_state (tab, state);

    if ((state == XED_TAB_STATE_LOADING_ERROR) || /* FIXME: add other states if needed */
        (state == XED_TAB_STATE_SHOWING_PRINT_PREVIEW))
    {
        gtk_widget_hide (GTK_WIDGET (tab->priv->frame));
    }
    else
    {
        if (tab->priv->print_preview == NULL)
        {
            gtk_widget_show (GTK_WIDGET (tab->priv->frame));
        }
    }

    set_cursor_according_to_state (GTK_TEXT_VIEW (xed_view_frame_get_view (tab->priv->frame)), state);

    update_auto_save_timeout (tab);

    g_object_notify (G_OBJECT (tab), "state");
    g_object_notify (G_OBJECT (tab), "can-close");
}

static void
document_location_notify_handler (GtkSourceFile *file,
                                  GParamSpec    *pspec,
                                  XedTab        *tab)
{
    xed_debug (DEBUG_TAB);

    /* Notify the change in the location */
    g_object_notify (G_OBJECT (tab), "name");
}

static void
document_shortname_notify_handler (XedDocument *document,
                                   GParamSpec  *pspec,
                                   XedTab      *tab)
{
    xed_debug (DEBUG_TAB);

    /* Notify the change in the shortname */
    g_object_notify (G_OBJECT (tab), "name");
}

static void
document_modified_changed (GtkTextBuffer *document,
                           XedTab        *tab)
{
    g_object_notify (G_OBJECT (tab), "name");
    g_object_notify (G_OBJECT (tab), "can-close");
}

static void
set_info_bar (XedTab    *tab,
              GtkWidget *info_bar)
{
    if (tab->priv->info_bar == info_bar)
    {
        return;
    }

    if (tab->priv->info_bar != NULL)
    {
        gtk_widget_destroy (tab->priv->info_bar);
    }

    tab->priv->info_bar = info_bar;

    if (info_bar == NULL)
    {
        return;
    }

    gtk_box_pack_start (GTK_BOX (tab), tab->priv->info_bar, FALSE, FALSE, 0);

    g_object_add_weak_pointer (G_OBJECT (tab->priv->info_bar), (gpointer *)&tab->priv->info_bar);
}

static void
remove_tab (XedTab *tab)
{
    XedNotebook *notebook;

    notebook = XED_NOTEBOOK (gtk_widget_get_parent (GTK_WIDGET (tab)));

    xed_notebook_remove_tab (notebook, tab);
}

static void
io_loading_error_info_bar_response (GtkWidget *info_bar,
                                    gint       response_id,
                                    XedTab    *tab)
{
    XedView *view;
    GFile *location;
    const GtkSourceEncoding *encoding;

    g_return_if_fail (tab->priv->loader != NULL);

    view = xed_tab_get_view (tab);
    location = gtk_source_file_loader_get_location (tab->priv->loader);

    switch (response_id)
    {
        case GTK_RESPONSE_OK:
            encoding = xed_conversion_error_info_bar_get_encoding (GTK_WIDGET (info_bar));

            set_info_bar (tab, NULL);
            xed_tab_set_state (tab, XED_TAB_STATE_LOADING);

            load (tab, encoding, tab->priv->tmp_line_pos);
            break;

        case GTK_RESPONSE_YES:
            /* This means that we want to edit the document anyway */
            tab->priv->editable = TRUE;
            gtk_text_view_set_editable (GTK_TEXT_VIEW (view), TRUE);
            set_info_bar (tab, NULL);
            clear_loading (tab);
            break;

        default:
            _xed_recent_remove (XED_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (tab))), location);

            remove_tab (tab);
            break;
    }
}

static void
file_already_open_warning_info_bar_response (GtkWidget *info_bar,
                                             gint       response_id,
                                             XedTab    *tab)
{
    XedView *view;

    view = xed_tab_get_view (tab);

    if (response_id == GTK_RESPONSE_YES)
    {
        tab->priv->editable = TRUE;
        gtk_text_view_set_editable (GTK_TEXT_VIEW (view), TRUE);
    }

    gtk_widget_destroy (info_bar);

    gtk_widget_grab_focus (GTK_WIDGET (view));
}

static void
load_cancelled (GtkWidget *area,
                gint       response_id,
                XedTab    *tab)
{
    g_return_if_fail (XED_IS_PROGRESS_INFO_BAR (tab->priv->info_bar));
    g_return_if_fail (G_IS_CANCELLABLE (tab->priv->cancellable));

    g_cancellable_cancel (tab->priv->cancellable);
}

static void
unrecoverable_reverting_error_info_bar_response (GtkWidget *info_bar,
                                                 gint       response_id,
                                                 XedTab    *tab)
{
    XedView *view;

    xed_tab_set_state (tab, XED_TAB_STATE_NORMAL);

    set_info_bar (tab, NULL);

    clear_loading (tab);

    view = xed_tab_get_view (tab);
    gtk_widget_grab_focus (GTK_WIDGET (view));
}

#define MAX_MSG_LENGTH 100

static void
show_loading_info_bar (XedTab *tab)
{
    GtkWidget *bar;
    XedDocument *doc = NULL;
    gchar *name;
    gchar *dirname = NULL;
    gchar *msg = NULL;
    gchar *name_markup;
    gchar *dirname_markup;
    gint len;

    if (tab->priv->info_bar != NULL)
    {
        return;
    }

    xed_debug (DEBUG_TAB);

    doc = xed_tab_get_document (tab);
    g_return_if_fail (doc != NULL);

    name = xed_document_get_short_name_for_display (doc);
    len = g_utf8_strlen (name, -1);

    /* if the name is awfully long, truncate it and be done with it,
     * otherwise also show the directory (ellipsized if needed)
     */
    if (len > MAX_MSG_LENGTH)
    {
        gchar *str;

        str = xed_utils_str_middle_truncate (name, MAX_MSG_LENGTH);
        g_free (name);
        name = str;
    }
    else
    {
        GtkSourceFile *file = xed_document_get_file (doc);
        GFile *location = gtk_source_file_get_location (file);

        if (location != NULL)
        {
            gchar *str = xed_utils_location_get_dirname_for_display (location);

            /* use the remaining space for the dir, but use a min of 20 chars
             * so that we do not end up with a dirname like "(a...b)".
             * This means that in the worst case when the filename is long 99
             * we have a title long 99 + 20, but I think it's a rare enough
             * case to be acceptable. It's justa darn title afterall :)
             */
            dirname = xed_utils_str_middle_truncate (str, MAX (20, MAX_MSG_LENGTH - len));
            g_free (str);
        }
    }

    name_markup = g_markup_printf_escaped ("<b>%s</b>", name);

    if (tab->priv->state == XED_TAB_STATE_REVERTING)
    {
        if (dirname != NULL)
        {
            dirname_markup = g_markup_printf_escaped ("<b>%s</b>", dirname);

            /* Translators: the first %s is a file name (e.g. test.txt) the second one
               is a directory (e.g. ssh://master.gnome.org/home/users/paolo) */
            msg = g_strdup_printf (_("Reverting %s from %s"), name_markup, dirname_markup);
            g_free (dirname_markup);
        }
        else
        {
            msg = g_strdup_printf (_("Reverting %s"), name_markup);
        }

        bar = xed_progress_info_bar_new ("document-revert-symbolic", msg, TRUE);
    }
    else
    {
        if (dirname != NULL)
        {
            dirname_markup = g_markup_printf_escaped ("<b>%s</b>", dirname);

            /* Translators: the first %s is a file name (e.g. test.txt) the second one
               is a directory (e.g. ssh://master.gnome.org/home/users/paolo) */
            msg = g_strdup_printf (_("Loading %s from %s"), name_markup, dirname_markup);
            g_free (dirname_markup);
        }
        else
        {
            msg = g_strdup_printf (_("Loading %s"), name_markup);
        }

        bar = xed_progress_info_bar_new ("document-open-symbolic", msg, TRUE);
    }

    g_signal_connect (bar, "response",
                      G_CALLBACK (load_cancelled), tab);

    gtk_widget_show (bar);

    set_info_bar (tab, bar);

    g_free (msg);
    g_free (name);
    g_free (name_markup);
    g_free (dirname);
}

static void
show_saving_info_bar (XedTab *tab)
{
    GtkWidget *bar;
    XedDocument *doc = NULL;
    gchar *short_name;
    gchar *from;
    gchar *to = NULL;
    gchar *from_markup;
    gchar *to_markup;
    gchar *msg = NULL;
    gint len;

    g_return_if_fail (tab->priv->task_saver != NULL);

    if (tab->priv->info_bar != NULL)
    {
        return;
    }

    xed_debug (DEBUG_TAB);

    doc = xed_tab_get_document (tab);
    g_return_if_fail (doc != NULL);

    short_name = xed_document_get_short_name_for_display (doc);

    len = g_utf8_strlen (short_name, -1);

    /* if the name is awfully long, truncate it and be done with it,
     * otherwise also show the directory (ellipsized if needed)
     */
    if (len > MAX_MSG_LENGTH)
    {
        from = xed_utils_str_middle_truncate (short_name, MAX_MSG_LENGTH);
        g_free (short_name);
    }
    else
    {
        gchar *str;
        SaverData *data;
        GFile *location;

        data = g_task_get_task_data (tab->priv->task_saver);
        location = gtk_source_file_saver_get_location (data->saver);

        from = short_name;
        to = g_file_get_parse_name (location);
        str = xed_utils_str_middle_truncate (to, MAX (20, MAX_MSG_LENGTH - len));
        g_free (to);

        to = str;
    }

    from_markup = g_markup_printf_escaped ("<b>%s</b>", from);

    if (to != NULL)
    {
        to_markup = g_markup_printf_escaped ("<b>%s</b>", to);

        /* Translators: the first %s is a file name (e.g. test.txt) the second one
           is a directory (e.g. ssh://master.gnome.org/home/users/paolo) */
        msg = g_strdup_printf (_("Saving %s to %s"), from_markup, to_markup);
        g_free (to_markup);
    }
    else
    {
        msg = g_strdup_printf (_("Saving %s"), from_markup);
    }

    bar = xed_progress_info_bar_new ("document-save-symbolic", msg, FALSE);

    gtk_widget_show (bar);

    set_info_bar (tab, bar);

    g_free (msg);
    g_free (to);
    g_free (from);
    g_free (from_markup);
}

static void
info_bar_set_progress (XedTab  *tab,
                       goffset  size,
                       goffset  total_size)
{
    if (tab->priv->info_bar == NULL)
    {
        return;
    }

    xed_debug_message (DEBUG_TAB, "%" G_GUINT64_FORMAT "/%" G_GUINT64_FORMAT, size, total_size);

    g_return_if_fail (XED_IS_PROGRESS_INFO_BAR (tab->priv->info_bar));

    if (total_size == 0)
    {
        if (size != 0)
            xed_progress_info_bar_pulse (XED_PROGRESS_INFO_BAR (tab->priv->info_bar));
        else
            xed_progress_info_bar_set_fraction (XED_PROGRESS_INFO_BAR (tab->priv->info_bar), 0);
    }
    else
    {
        gdouble frac;

        frac = (gdouble)size / (gdouble)total_size;

        xed_progress_info_bar_set_fraction (XED_PROGRESS_INFO_BAR (tab->priv->info_bar), frac);
    }
}

static gboolean
scroll_to_cursor (XedTab *tab)
{
    XedView *view;

    view = xed_tab_get_view (tab);
    xed_view_scroll_to_cursor (view);

    tab->priv->idle_scroll = 0;
    return G_SOURCE_REMOVE;
}

static void
unrecoverable_saving_error_info_bar_response (GtkWidget *info_bar,
                                              gint       response_id,
                                              XedTab    *tab)
{
    XedView *view;

    if (tab->priv->print_preview != NULL)
    {
        xed_tab_set_state (tab, XED_TAB_STATE_SHOWING_PRINT_PREVIEW);
    }
    else
    {
        xed_tab_set_state (tab, XED_TAB_STATE_NORMAL);
    }

    set_info_bar (tab, NULL);

    g_return_if_fail (tab->priv->task_saver != NULL);
    g_task_return_boolean (tab->priv->task_saver, FALSE);

    view = xed_tab_get_view (tab);

    gtk_widget_grab_focus (GTK_WIDGET (view));
}

/* Sets the save flags after an info bar response. */
static void
response_set_save_flags (XedTab                  *tab,
                         GtkSourceFileSaverFlags  save_flags)
{
    SaverData *data;
    gboolean create_backup;

    data = g_task_get_task_data (tab->priv->task_saver);

    create_backup = g_settings_get_boolean (tab->priv->editor, XED_SETTINGS_CREATE_BACKUP_COPY);

    /* If we are here, it means that the user expressed his or her willing
    * to save the file, by pressing a button in the info bar. So even if
    * the file saving was initially an auto-save, we set the create_backup
    * flag (if the conditions are met).
    */
    if (create_backup && !data->force_no_backup)
    {
        save_flags |= GTK_SOURCE_FILE_SAVER_FLAGS_CREATE_BACKUP;
    }
    else
    {
        save_flags &= ~GTK_SOURCE_FILE_SAVER_FLAGS_CREATE_BACKUP;
    }

    gtk_source_file_saver_set_flags (data->saver, save_flags);
}

static void
invalid_character_info_bar_response (GtkWidget *info_bar,
                                     gint       response_id,
                                     XedTab    *tab)
{
    if (response_id == GTK_RESPONSE_YES)
    {
        SaverData *data;
        GtkSourceFileSaverFlags save_flags;

        set_info_bar (tab, NULL);

        g_return_if_fail (tab->priv->task_saver != NULL);
        data = g_task_get_task_data (tab->priv->task_saver);

        /* Don't bug the user again with this... */
        tab->priv->save_flags |= GTK_SOURCE_FILE_SAVER_FLAGS_IGNORE_INVALID_CHARS;

        save_flags = gtk_source_file_saver_get_flags (data->saver);
        save_flags |= GTK_SOURCE_FILE_SAVER_FLAGS_IGNORE_INVALID_CHARS;
        response_set_save_flags (tab, save_flags);

        /* Force saving */
        save (tab);
    }
    else
    {
        unrecoverable_saving_error_info_bar_response (info_bar, response_id, tab);
    }
}

static void
no_backup_error_info_bar_response (GtkWidget *info_bar,
                                   gint       response_id,
                                   XedTab    *tab)
{
    if (response_id == GTK_RESPONSE_YES)
    {
        SaverData *data;
        GtkSourceFileSaverFlags save_flags;

        set_info_bar (tab, NULL);

        g_return_if_fail (tab->priv->task_saver != NULL);
        data = g_task_get_task_data (tab->priv->task_saver);

        data->force_no_backup = TRUE;
        save_flags = gtk_source_file_saver_get_flags (data->saver);
        response_set_save_flags (tab, save_flags);

        /* Force saving */
        save (tab);
    }
    else
    {
        unrecoverable_saving_error_info_bar_response (info_bar, response_id, tab);
    }
}

static void
externally_modified_error_info_bar_response (GtkWidget *info_bar,
                                             gint       response_id,
                                             XedTab    *tab)
{
    if (response_id == GTK_RESPONSE_YES)
    {
        SaverData *data;
        GtkSourceFileSaverFlags save_flags;

        set_info_bar (tab, NULL);

        g_return_if_fail (tab->priv->task_saver != NULL);
        data = g_task_get_task_data (tab->priv->task_saver);

        /* ignore_modification_time should not be persisted in save
         * flags across saves (i.e. priv->save_flags is not modified).
         */

        save_flags = gtk_source_file_saver_get_flags (data->saver);
        save_flags |= GTK_SOURCE_FILE_SAVER_FLAGS_IGNORE_MODIFICATION_TIME;
        response_set_save_flags (tab, save_flags);

        /* Force saving */
        save (tab);
    }
    else
    {
        unrecoverable_saving_error_info_bar_response (info_bar, response_id, tab);
    }
}

static void
recoverable_saving_error_info_bar_response (GtkWidget *info_bar,
                                            gint       response_id,
                                            XedTab    *tab)
{
    if (response_id == GTK_RESPONSE_OK)
    {
        SaverData *data;
        const GtkSourceEncoding *encoding;

        set_info_bar (tab, NULL);

        g_return_if_fail (tab->priv->task_saver != NULL);
        data = g_task_get_task_data (tab->priv->task_saver);

        encoding = xed_conversion_error_info_bar_get_encoding (GTK_WIDGET (info_bar));
        g_return_if_fail (encoding != NULL);

        gtk_source_file_saver_set_encoding (data->saver, encoding);
        save (tab);
    }
    else
    {
        unrecoverable_saving_error_info_bar_response (info_bar, response_id, tab);
    }
}

static void
externally_modified_notification_info_bar_response (GtkWidget *info_bar,
                                                    gint       response_id,
                                                    XedTab    *tab)
{
    XedView *view;

    set_info_bar (tab, NULL);
    view = xed_tab_get_view (tab);

    if (response_id == GTK_RESPONSE_OK)
    {
        _xed_tab_revert (tab);
    }
    else
    {
        tab->priv->ask_if_externally_modified = FALSE;

        /* go back to normal state */
        xed_tab_set_state (tab, XED_TAB_STATE_NORMAL);
    }

    gtk_widget_grab_focus (GTK_WIDGET (view));
}

static void
display_externally_modified_notification (XedTab *tab)
{
    GtkWidget *info_bar;
    XedDocument *doc;
    GtkSourceFile *file;
    GFile *location;
    gboolean document_modified;

    doc = xed_tab_get_document (tab);
    g_return_if_fail (XED_IS_DOCUMENT (doc));
    file = xed_document_get_file (doc);

    /* we're here because the file we're editing changed on disk */
    location = gtk_source_file_get_location (file);
    g_return_if_fail (location != NULL);

    document_modified = gtk_text_buffer_get_modified (GTK_TEXT_BUFFER(doc));
    info_bar = xed_externally_modified_info_bar_new (location, document_modified);

    tab->priv->info_bar = NULL;
    set_info_bar (tab, info_bar);
    gtk_widget_show (info_bar);

    g_signal_connect (info_bar, "response",
                      G_CALLBACK (externally_modified_notification_info_bar_response), tab);
}

static gboolean
view_focused_in (GtkWidget     *widget,
                 GdkEventFocus *event,
                 XedTab        *tab)
{
    XedDocument *doc;
    GtkSourceFile *file;

    g_return_val_if_fail (XED_IS_TAB (tab), FALSE);

    /* we try to detect file changes only in the normal state */
    if (tab->priv->state != XED_TAB_STATE_NORMAL)
    {
        return GDK_EVENT_PROPAGATE;
    }

    /* we already asked, don't bug the user again */
    if (!tab->priv->ask_if_externally_modified)
    {
        return GDK_EVENT_PROPAGATE;
    }

    doc = xed_tab_get_document (tab);
    file = xed_document_get_file (doc);

    /* If file was never saved or is remote we do not check */
    if (gtk_source_file_is_local (file))
    {
        gtk_source_file_check_file_on_disk (file);

        if (gtk_source_file_is_externally_modified (file))
        {
            xed_tab_set_state (tab, XED_TAB_STATE_EXTERNALLY_MODIFIED_NOTIFICATION);

            display_externally_modified_notification (tab);
        }
    }

    return GDK_EVENT_PROPAGATE;
}

static void
xed_tab_init (XedTab *tab)
{
    gboolean auto_save;
    guint auto_save_interval;
    XedDocument *doc;
    XedView *view;
    GtkSourceFile *file;

    tab->priv = XED_TAB_GET_PRIVATE (tab);

    tab->priv->editor = g_settings_new ("org.x.editor.preferences.editor");
    tab->priv->state = XED_TAB_STATE_NORMAL;
    tab->priv->editable = TRUE;
    tab->priv->ask_if_externally_modified = TRUE;

    gtk_orientable_set_orientation (GTK_ORIENTABLE (tab), GTK_ORIENTATION_VERTICAL);

    /* Manage auto save data */
    auto_save = g_settings_get_boolean (tab->priv->editor, XED_SETTINGS_AUTO_SAVE);
    auto_save_interval = g_settings_get_uint (tab->priv->editor, XED_SETTINGS_AUTO_SAVE_INTERVAL);
    tab->priv->auto_save = auto_save;
    tab->priv->auto_save = (tab->priv->auto_save != FALSE);

    tab->priv->auto_save_interval = auto_save_interval;

    /* Create the frame */
    tab->priv->frame = xed_view_frame_new ();
    gtk_widget_show (GTK_WIDGET (tab->priv->frame));

    gtk_box_pack_end (GTK_BOX (tab), GTK_WIDGET (tab->priv->frame), TRUE, TRUE, 0);

    doc = xed_view_frame_get_document (tab->priv->frame);
    g_object_set_data (G_OBJECT (doc), XED_TAB_KEY, tab);

    view = xed_view_frame_get_view (tab->priv->frame);
    g_object_set_data (G_OBJECT (view), XED_TAB_KEY, tab);

    file = xed_document_get_file (doc);

    g_signal_connect_object (file, "notify::location",
                             G_CALLBACK (document_location_notify_handler), tab, 0);
    g_signal_connect (doc, "notify::shortname",
                      G_CALLBACK (document_shortname_notify_handler), tab);
    g_signal_connect (doc, "modified_changed",
                      G_CALLBACK (document_modified_changed), tab);

    g_signal_connect_after (view, "focus-in-event",
                            G_CALLBACK (view_focused_in), tab);
    g_signal_connect_after (view, "realize",
                            G_CALLBACK (view_realized), tab);
}

GtkWidget *
_xed_tab_new (void)
{
    return GTK_WIDGET (g_object_new (XED_TYPE_TAB, NULL));
}

/* Whether create is TRUE, creates a new empty document if location does
   not refer to an existing file */
GtkWidget *
_xed_tab_new_from_location (GFile                   *location,
                            const GtkSourceEncoding *encoding,
                            gint                     line_pos,
                            gboolean                 create)
{
    XedTab *tab;

    g_return_val_if_fail (G_IS_FILE (location), NULL);

    tab = XED_TAB (_xed_tab_new ());

    _xed_tab_load (tab, location, encoding, line_pos, create);

    return GTK_WIDGET (tab);
}

GtkWidget *
_xed_tab_new_from_stream (GInputStream            *stream,
                          const GtkSourceEncoding *encoding,
                          gint                     line_pos)
{
    GtkWidget *tab;

    g_return_val_if_fail (G_IS_INPUT_STREAM (stream), NULL);

    tab = _xed_tab_new ();

    _xed_tab_load_stream (XED_TAB (tab), stream, encoding, line_pos);

    return tab;
}

/**
 * xed_tab_get_view:
 * @tab: a #XedTab
 *
 * Gets the #XedView inside @tab.
 *
 * Returns: (transfer none): the #XedView inside @tab
 */
XedView *
xed_tab_get_view (XedTab *tab)
{
    g_return_val_if_fail (XED_IS_TAB (tab), NULL);

    return xed_view_frame_get_view (tab->priv->frame);
}

/**
 * xed_tab_get_document:
 * @tab: a #XedTab
 *
 * Gets the #XedDocument associated to @tab.
 *
 * Returns: (transfer none): the #XedDocument associated to @tab
 */
XedDocument *
xed_tab_get_document (XedTab *tab)
{
    g_return_val_if_fail (XED_IS_TAB (tab), NULL);

    return xed_view_frame_get_document (tab->priv->frame);
}

#define MAX_DOC_NAME_LENGTH 40

gchar *
_xed_tab_get_name (XedTab *tab)
{
    XedDocument *doc;
    gchar *name;
    gchar *docname;
    gchar *tab_name;

    g_return_val_if_fail (XED_IS_TAB (tab), NULL);

    doc = xed_tab_get_document (tab);

    name = xed_document_get_short_name_for_display (doc);

    /* Truncate the name so it doesn't get insanely wide. */
    docname = xed_utils_str_middle_truncate (name, MAX_DOC_NAME_LENGTH);

    if (gtk_text_buffer_get_modified (GTK_TEXT_BUFFER (doc)))
    {
        tab_name = g_strdup_printf ("*%s", docname);
    }
    else
    {
 #if 0
        if (xed_document_get_readonly (doc))
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
_xed_tab_get_tooltips (XedTab *tab)
{
    XedDocument *doc;
    gchar *tip;
    gchar *uri;
    gchar *ruri;
    gchar *ruri_markup;

    g_return_val_if_fail (XED_IS_TAB (tab), NULL);

    doc = xed_tab_get_document (tab);

    uri = xed_document_get_uri_for_display (doc);
    g_return_val_if_fail (uri != NULL, NULL);

    ruri =  xed_utils_replace_home_dir_with_tilde (uri);
    g_free (uri);

    ruri_markup = g_markup_printf_escaped ("<i>%s</i>", ruri);

    switch (tab->priv->state)
    {
        gchar *content_type;
        gchar *mime_type;
        gchar *content_description;
        gchar *content_full_description;
        gchar *encoding;
        GtkSourceFile *file;
        const GtkSourceEncoding *enc;

        case XED_TAB_STATE_LOADING_ERROR:
            tip = g_strdup_printf (_("Error opening file %s"), ruri_markup);
            break;

        case XED_TAB_STATE_REVERTING_ERROR:
            tip = g_strdup_printf (_("Error reverting file %s"), ruri_markup);
            break;

        case XED_TAB_STATE_SAVING_ERROR:
            tip =  g_strdup_printf (_("Error saving file %s"), ruri_markup);
            break;
        default:
            content_type = xed_document_get_content_type (doc);
            mime_type = xed_document_get_mime_type (doc);
            content_description = g_content_type_get_description (content_type);

            if (content_description == NULL)
            {
                content_full_description = g_strdup (mime_type);
            }
            else
            {
                content_full_description = g_strdup_printf ("%s (%s)", content_description, mime_type);
            }

            g_free (content_type);
            g_free (mime_type);
            g_free (content_description);

            file = xed_document_get_file (doc);
            enc = gtk_source_file_get_encoding (file);

            if (enc == NULL)
            {
                enc = gtk_source_encoding_get_utf8 ();
            }

            encoding = gtk_source_encoding_to_string (enc);

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
get_icon (GtkIconTheme *theme,
          GFile        *location,
          gint          size)
{
    GdkPixbuf *pixbuf;
    GtkIconInfo *icon_info;
    GFileInfo *info;
    GIcon *gicon;

    if (location == NULL)
    {
        return gtk_icon_theme_load_icon (theme, "text-x-generic", size, 0, NULL);
    }

    /* FIXME: Doing a sync stat is bad, this should be fixed */
    info = g_file_query_info (location,
                              G_FILE_ATTRIBUTE_STANDARD_ICON,
                              G_FILE_QUERY_INFO_NONE,
                              NULL,
                              NULL);
    if (info == NULL)
    {
        return gtk_icon_theme_load_icon (theme, "text-x-generic", size, 0, NULL);
    }

    gicon = g_file_info_get_icon (info);

    if (gicon == NULL)
    {
        g_object_unref (info);
        return gtk_icon_theme_load_icon (theme, "text-x-generic", size, 0, NULL);
    }

    icon_info = gtk_icon_theme_lookup_by_gicon (theme, gicon, size, 0);
    g_object_unref (info);

    if (icon_info == NULL)
    {
        return gtk_icon_theme_load_icon (theme, "text-x-generic", size, 0, NULL);
    }

    pixbuf = gtk_icon_info_load_icon (icon_info, NULL);
    g_object_unref (icon_info);

    if (pixbuf == NULL)
    {
        return gtk_icon_theme_load_icon (theme, "text-x-generic", size, 0, NULL);
    }

    return pixbuf;
}

/* FIXME: add support for theme changed. I think it should be as easy as
   call g_object_notify (tab, "name") when the icon theme changes */
GdkPixbuf *
_xed_tab_get_icon (XedTab *tab)
{
    GdkScreen *screen;
    GtkIconTheme *theme;
    gint icon_size;
    const gchar *icon_name;
    GdkPixbuf *pixbuf = NULL;

    g_return_val_if_fail (XED_IS_TAB (tab), NULL);

    screen = gtk_widget_get_screen (GTK_WIDGET (tab));
    theme = gtk_icon_theme_get_for_screen (screen);
    g_return_val_if_fail (theme != NULL, NULL);

    gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, NULL, &icon_size);

    switch (tab->priv->state)
    {
        case XED_TAB_STATE_LOADING:
            icon_name = "document-open-symbolic";
            break;

        case XED_TAB_STATE_REVERTING:
            icon_name = "document-revert-symbolic";
            break;

        case XED_TAB_STATE_SAVING:
            icon_name = "document-save-symbolic";
            break;

        case XED_TAB_STATE_PRINTING:
            icon_name = "printer-printing-symbolic";
            break;

        case XED_TAB_STATE_PRINT_PREVIEWING:
        case XED_TAB_STATE_SHOWING_PRINT_PREVIEW:
            icon_name = "printer-symbolic";
            break;

        case XED_TAB_STATE_LOADING_ERROR:
        case XED_TAB_STATE_REVERTING_ERROR:
        case XED_TAB_STATE_SAVING_ERROR:
        case XED_TAB_STATE_GENERIC_ERROR:
            icon_name = "dialog-error-symbolic";
            break;

        case XED_TAB_STATE_EXTERNALLY_MODIFIED_NOTIFICATION:
            icon_name = "dialog-warning-symbolic";
            break;

        default:
            icon_name = NULL;
    }

    if (icon_name != NULL)
    {
        pixbuf = gtk_icon_theme_load_icon (theme, icon_name, icon_size, 0, NULL);
    }
    else
    {
        GFile *location;
        XedDocument *doc;

        doc = xed_tab_get_document (tab);
        location = xed_document_get_location (doc);
        pixbuf = get_icon (theme, location, icon_size);
    }

    return pixbuf;
}

/**
 * xed_tab_get_from_document:
 * @doc: a #XedDocument
 *
 * Gets the #XedTab associated with @doc.
 *
 * Returns: (transfer none): the #XedTab associated with @doc
 */
XedTab *
xed_tab_get_from_document (XedDocument *doc)
{
    gpointer res;

    g_return_val_if_fail (XED_IS_DOCUMENT (doc), NULL);

    res = g_object_get_data (G_OBJECT (doc), XED_TAB_KEY);

    return (res != NULL) ? XED_TAB (res) : NULL;
}

static void
loader_progress_cb (goffset  size,
                    goffset  total_size,
                    XedTab  *tab)
{
    gdouble elapsed_time;
    gdouble total_time;
    gdouble remaining_time;

    g_return_if_fail (tab->priv->state == XED_TAB_STATE_LOADING ||
                      tab->priv->state == XED_TAB_STATE_REVERTING);

    if (tab->priv->timer == NULL)
    {
        tab->priv->timer = g_timer_new ();
    }

    elapsed_time = g_timer_elapsed (tab->priv->timer, NULL);

    /* elapsed_time / total_time = size / total_size */
    total_time = (elapsed_time * total_size) / size;

    remaining_time = total_time - elapsed_time;

    /* Approximately more than 3 seconds remaining. */
    if (remaining_time > 3.0)
    {
        show_loading_info_bar (tab);
    }

    info_bar_set_progress (tab, size, total_size);
}

static void
goto_line (XedTab *tab)
{
    XedDocument *doc = xed_tab_get_document (tab);
    GtkTextIter iter;

    /* Move the cursor at the requested line if any. */
    if (tab->priv->tmp_line_pos > 0)
    {
        xed_document_goto_line_offset (doc, tab->priv->tmp_line_pos - 1, 0);
        return;
    }

    /* If enabled, move to the position stored in the metadata. */
    if (g_settings_get_boolean (tab->priv->editor, XED_SETTINGS_RESTORE_CURSOR_POSITION))
    {
        gchar *pos;
        gint offset;

        pos = xed_document_get_metadata (doc, XED_METADATA_ATTRIBUTE_POSITION);

        offset = pos != NULL ? atoi (pos) : 0;
        g_free (pos);

        gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (doc), &iter, MAX (0, offset));

        /* make sure it's a valid position, if the file
        * changed we may have ended up in the middle of
        * a utf8 character cluster */
        if (!gtk_text_iter_is_cursor_position (&iter))
        {
            gtk_text_iter_set_line_offset (&iter, 0);
        }
    }

    /* Otherwise to the top. */
    else
    {
        gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (doc), &iter);
    }

    gtk_text_buffer_place_cursor (GTK_TEXT_BUFFER (doc), &iter);
}

static void
load_cb (GtkSourceFileLoader *loader,
         GAsyncResult        *result,
         XedTab              *tab)
{
    XedDocument *doc = xed_tab_get_document (tab);
    GFile *location = gtk_source_file_loader_get_location (loader);
    gboolean create_named_new_doc;
    GError *error = NULL;

    g_return_if_fail (tab->priv->state == XED_TAB_STATE_LOADING ||
                      tab->priv->state == XED_TAB_STATE_REVERTING);

    gtk_source_file_loader_load_finish (loader, result, &error);

    if (error != NULL)
    {
        xed_debug_message (DEBUG_TAB, "File loading error: %s", error->message);
    }

    if (tab->priv->timer != NULL)
    {
        g_timer_destroy (tab->priv->timer);
        tab->priv->timer = NULL;
    }

    set_info_bar (tab, NULL);

    /* Load was successful. */
    if (error == NULL ||
       (error->domain == GTK_SOURCE_FILE_LOADER_ERROR &&
        error->code == GTK_SOURCE_FILE_LOADER_ERROR_CONVERSION_FALLBACK))
    {
        if (tab->priv->user_requested_encoding)
        {
            const GtkSourceEncoding *encoding = gtk_source_file_loader_get_encoding (loader);
            const gchar *charset = gtk_source_encoding_get_charset (encoding);

            xed_document_set_metadata (doc, XED_METADATA_ATTRIBUTE_ENCODING, charset, NULL);
        }

        goto_line (tab);
    }

    /* Special case creating a named new doc. */
    create_named_new_doc = (_xed_document_get_create (doc) &&
                            error != NULL &&
                            error->domain == G_IO_ERROR &&
                            error->code == G_IO_ERROR_NOT_FOUND &&
                            g_file_has_uri_scheme (location, "file"));

    if (create_named_new_doc)
    {
        g_error_free (error);
        error = NULL;
    }

    /* If the error is CONVERSION FALLBACK don't treat it as a normal error. */
    if (error != NULL &&
       (error->domain != GTK_SOURCE_FILE_LOADER_ERROR ||
        error->code != GTK_SOURCE_FILE_LOADER_ERROR_CONVERSION_FALLBACK))
    {
        if (tab->priv->state == XED_TAB_STATE_LOADING)
        {
            xed_tab_set_state (tab, XED_TAB_STATE_LOADING_ERROR);
        }
        else
        {
            xed_tab_set_state (tab, XED_TAB_STATE_REVERTING_ERROR);
        }

        if (error->domain == G_IO_ERROR && error->code == G_IO_ERROR_CANCELLED)
        {
            remove_tab (tab);
        }
        else
        {
            GtkWidget *info_bar;

            if (location != NULL)
            {
                _xed_recent_remove (XED_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (tab))), location);
            }

            if (tab->priv->state == XED_TAB_STATE_LOADING_ERROR)
            {
                const GtkSourceEncoding *encoding;

                encoding = gtk_source_file_loader_get_encoding (loader);

                info_bar = xed_io_loading_error_info_bar_new (location, encoding, error);

                g_signal_connect (info_bar, "response",
                                  G_CALLBACK (io_loading_error_info_bar_response), tab);
            }
            else
            {
                g_return_if_fail (tab->priv->state == XED_TAB_STATE_REVERTING_ERROR);

                info_bar = xed_unrecoverable_reverting_error_info_bar_new (location, error);

                g_signal_connect (info_bar, "response",
                                  G_CALLBACK (unrecoverable_reverting_error_info_bar_response), tab);
            }

            set_info_bar (tab, info_bar);
            gtk_info_bar_set_default_response (GTK_INFO_BAR (info_bar), GTK_RESPONSE_CANCEL);
            gtk_widget_show (info_bar);
        }

        goto end;
    }

    if (location != NULL && !create_named_new_doc)
    {
        gchar *mime = xed_document_get_mime_type (doc);

        _xed_recent_add (XED_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (tab))), location, mime);
        g_free (mime);
    }

    if (error != NULL &&
        error->domain == GTK_SOURCE_FILE_LOADER_ERROR &&
        error->code == GTK_SOURCE_FILE_LOADER_ERROR_CONVERSION_FALLBACK)
    {
        GtkWidget *info_bar;
        const GtkSourceEncoding *encoding;

        /* Set the tab as not editable as we have an error, the user can
        * decide to make it editable again.
        */
        tab->priv->editable = FALSE;

        encoding = gtk_source_file_loader_get_encoding (loader);

        info_bar = xed_io_loading_error_info_bar_new (location, encoding, error);

        g_signal_connect (info_bar, "response",
                         G_CALLBACK (io_loading_error_info_bar_response), tab);

        set_info_bar (tab, info_bar);
        gtk_info_bar_set_default_response (GTK_INFO_BAR (info_bar), GTK_RESPONSE_CANCEL);
        gtk_widget_show (info_bar);
    }

    /* Scroll to the cursor when the document is loaded, we need to do it in
    * an idle as after the document is loaded the textview is still
    * redrawing and relocating its internals.
    */
    if (tab->priv->idle_scroll == 0)
    {
       tab->priv->idle_scroll = g_idle_add ((GSourceFunc)scroll_to_cursor, tab);
    }

    /* If the document is readonly we don't care how many times the document
    * is opened.
    */
    if (!xed_document_get_readonly (doc))
    {
        GList *all_documents;
        GList *l;

        all_documents = xed_app_get_documents (XED_APP (g_application_get_default ()));

        for (l = all_documents; l != NULL; l = g_list_next (l))
        {
            XedDocument *cur_doc = l->data;

            if (cur_doc != doc)
            {
                GtkSourceFile *cur_file = xed_document_get_file (cur_doc);
                GFile *cur_location = gtk_source_file_get_location (cur_file);

                if (cur_location != NULL && location != NULL && g_file_equal (location, cur_location))
                {
                    GtkWidget *info_bar;

                    tab->priv->editable = FALSE;

                    info_bar = xed_file_already_open_warning_info_bar_new (location);

                    g_signal_connect (info_bar, "response",
                                      G_CALLBACK (file_already_open_warning_info_bar_response), tab);

                    set_info_bar (tab, info_bar);
                    gtk_info_bar_set_default_response (GTK_INFO_BAR (info_bar), GTK_RESPONSE_CANCEL);
                    gtk_widget_show (info_bar);

                    break;
                }
            }
        }

       g_list_free (all_documents);
    }

    xed_tab_set_state (tab, XED_TAB_STATE_NORMAL);

    if (location == NULL)
    {
       /* FIXME: hackish */
       gtk_text_buffer_set_modified (GTK_TEXT_BUFFER (doc), TRUE);
    }

    tab->priv->ask_if_externally_modified = TRUE;

    if (error == NULL)
    {
       clear_loading (tab);
    }

    g_signal_emit_by_name (doc, "loaded");

end:
    /* Async operation finished. */
    g_object_unref (tab);

    if (error != NULL)
    {
        g_error_free (error);
    }
}

static GSList *
get_candidate_encodings (XedTab *tab)
{
    XedDocument *doc;
    GSettings *enc_settings;
    gchar **enc_strv;
    gchar *metadata_charset;
    GtkSourceFile *file;
    const GtkSourceEncoding *file_encoding;
    GSList *encodings;

    enc_settings = g_settings_new ("org.x.editor.preferences.encodings");

    enc_strv = g_settings_get_strv (enc_settings, XED_SETTINGS_ENCODING_AUTO_DETECTED);

    encodings = _xed_utils_encoding_strv_to_list ((const gchar * const *)enc_strv);

    doc = xed_tab_get_document (tab);
    metadata_charset = xed_document_get_metadata (doc, XED_METADATA_ATTRIBUTE_ENCODING);

    if (metadata_charset != NULL)
    {
        const GtkSourceEncoding *metadata_enc;

        metadata_enc = gtk_source_encoding_get_from_charset (metadata_charset);

        if (metadata_enc != NULL)
        {
            encodings = g_slist_prepend (encodings, (gpointer)metadata_enc);
        }
    }

    file = xed_document_get_file (doc);
    file_encoding = gtk_source_file_get_encoding (file);

    if (file_encoding != NULL)
    {
        encodings = g_slist_prepend (encodings, (gpointer)file_encoding);
    }

    g_object_unref (enc_settings);
    g_strfreev (enc_strv);
    g_free (metadata_charset);

    return encodings;
}

static void
load (XedTab                  *tab,
      const GtkSourceEncoding *encoding,
      gint                     line_pos)
{
    GSList *candidate_encodings = NULL;
    XedDocument *doc;

    g_return_if_fail (GTK_SOURCE_IS_FILE_LOADER (tab->priv->loader));

    if (encoding != NULL)
    {
        tab->priv->user_requested_encoding = TRUE;
        candidate_encodings = g_slist_append (NULL, (gpointer) encoding);
    }
    else
    {
        tab->priv->user_requested_encoding = FALSE;
        candidate_encodings = get_candidate_encodings (tab);
    }

    gtk_source_file_loader_set_candidate_encodings (tab->priv->loader, candidate_encodings);
    g_slist_free (candidate_encodings);

    tab->priv->tmp_line_pos = line_pos;

    g_clear_object (&tab->priv->cancellable);
    tab->priv->cancellable = g_cancellable_new ();

    doc = xed_tab_get_document (tab);
    g_signal_emit_by_name (doc, "load");

    /* Keep the tab alive during the async operation. */
    g_object_ref (tab);

    gtk_source_file_loader_load_async (tab->priv->loader,
                                       G_PRIORITY_DEFAULT,
                                       tab->priv->cancellable,
                                       (GFileProgressCallback) loader_progress_cb,
                                       tab,
                                       NULL,
                                       (GAsyncReadyCallback) load_cb,
                                       tab);
}

void
_xed_tab_load (XedTab                  *tab,
               GFile                   *location,
               const GtkSourceEncoding *encoding,
               gint                     line_pos,
               gboolean                 create)
{
    XedDocument *doc;
    GtkSourceFile *file;

    g_return_if_fail (XED_IS_TAB (tab));
    g_return_if_fail (G_IS_FILE (location));
    g_return_if_fail (tab->priv->state == XED_TAB_STATE_NORMAL);

    xed_tab_set_state (tab, XED_TAB_STATE_LOADING);

    doc = xed_tab_get_document (tab);
    file = xed_document_get_file (doc);

    if (tab->priv->loader != NULL)
    {
        g_warning ("XedTab: file loader already exists.");
        g_object_unref (tab->priv->loader);
    }

    gtk_source_file_set_location (file, location);
    tab->priv->loader = gtk_source_file_loader_new (GTK_SOURCE_BUFFER (doc), file);

    _xed_document_set_create (doc, create);

    load (tab, encoding, line_pos);
}

void
_xed_tab_load_stream (XedTab                  *tab,
                      GInputStream            *stream,
                      const GtkSourceEncoding *encoding,
                      gint                     line_pos)
{
    XedDocument *doc;
    GtkSourceFile *file;

    g_return_if_fail (XED_IS_TAB (tab));
    g_return_if_fail (G_IS_INPUT_STREAM (stream));
    g_return_if_fail (tab->priv->state == XED_TAB_STATE_NORMAL);

    xed_tab_set_state (tab, XED_TAB_STATE_LOADING);

    doc = xed_tab_get_document (tab);
    file = xed_document_get_file (doc);

    if (tab->priv->loader != NULL)
    {
        g_warning ("XedTab: file loader already exists.");
        g_object_unref (tab->priv->loader);
    }

    gtk_source_file_set_location (file, NULL);

    tab->priv->loader = gtk_source_file_loader_new_from_stream (GTK_SOURCE_BUFFER (doc), file, stream);

    _xed_document_set_create (doc, FALSE);

    load (tab, encoding, line_pos);
}

void
_xed_tab_revert (XedTab *tab)
{
    XedDocument *doc;
    GtkSourceFile *file;
    GFile *location;

    g_return_if_fail (XED_IS_TAB (tab));
    g_return_if_fail (tab->priv->state == XED_TAB_STATE_NORMAL ||
                      tab->priv->state == XED_TAB_STATE_EXTERNALLY_MODIFIED_NOTIFICATION);

    if (tab->priv->state == XED_TAB_STATE_EXTERNALLY_MODIFIED_NOTIFICATION)
    {
        set_info_bar (tab, NULL);
    }

    doc = xed_tab_get_document (tab);
    file = xed_document_get_file (doc);
    location = gtk_source_file_get_location (file);
    g_return_if_fail (location != NULL);

    xed_tab_set_state (tab, XED_TAB_STATE_REVERTING);

    if (tab->priv->loader != NULL)
    {
        g_warning ("XedTab: file loader already exists.");
        g_object_unref (tab->priv->loader);
    }

    tab->priv->loader = gtk_source_file_loader_new (GTK_SOURCE_BUFFER (doc), file);

    load (tab, NULL, 0);
}

static void
saver_progress_cb (goffset size,
                   goffset total_size,
                   XedTab *tab)
{
    gdouble elapsed_time;
    gdouble total_time;
    gdouble remaining_time;

    g_return_if_fail (tab->priv->state == XED_TAB_STATE_SAVING);

    if (tab->priv->timer == NULL)
    {
        tab->priv->timer = g_timer_new ();
    }

    elapsed_time = g_timer_elapsed (tab->priv->timer, NULL);

    /* elapsed_time / total_time = size / total_size */
    total_time = (elapsed_time * total_size) / size;

    remaining_time = total_time - elapsed_time;

    /* Approximately more than 3 seconds remaining. */
    if (remaining_time > 3.0)
    {
        show_saving_info_bar (tab);
    }

    info_bar_set_progress (tab, size, total_size);
}

static void
save_cb (GtkSourceFileSaver *saver,
         GAsyncResult       *result,
         XedTab             *tab)
{
    XedDocument *doc = xed_tab_get_document (tab);
    GFile *location = gtk_source_file_saver_get_location (saver);
    GError *error = NULL;

    g_return_if_fail (tab->priv->task_saver != NULL);

    gtk_source_file_saver_save_finish (saver, result, &error);

    if (error != NULL)
    {
        xed_debug_message (DEBUG_TAB, "File saving error: %s", error->message);
    }

    if (tab->priv->timer != NULL)
    {
        g_timer_destroy (tab->priv->timer);
        tab->priv->timer = NULL;
    }

    set_info_bar (tab, NULL);

    if (error != NULL)
    {
        GtkWidget *info_bar;

        xed_tab_set_state (tab, XED_TAB_STATE_SAVING_ERROR);

        if (error->domain == GTK_SOURCE_FILE_SAVER_ERROR &&
            error->code == GTK_SOURCE_FILE_SAVER_ERROR_EXTERNALLY_MODIFIED)
        {
            /* This error is recoverable */
            info_bar = xed_externally_modified_saving_error_info_bar_new (location, error);
            g_return_if_fail (info_bar != NULL);

            g_signal_connect (info_bar, "response",
                              G_CALLBACK (externally_modified_error_info_bar_response), tab);
        }
        else if (error->domain == G_IO_ERROR &&
                 error->code == G_IO_ERROR_CANT_CREATE_BACKUP)
        {
            /* This error is recoverable */
            info_bar = xed_no_backup_saving_error_info_bar_new (location, error);
            g_return_if_fail (info_bar != NULL);

            g_signal_connect (info_bar, "response",
                              G_CALLBACK (no_backup_error_info_bar_response), tab);
        }
        else if (error->domain == GTK_SOURCE_FILE_SAVER_ERROR &&
                 error->code == GTK_SOURCE_FILE_SAVER_ERROR_INVALID_CHARS)
        {
            /* If we have any invalid char in the document we must warn the user
             * as it can make the document useless if it is saved.
             */
            info_bar = xed_invalid_character_info_bar_new (location);
            g_return_if_fail (info_bar != NULL);

            g_signal_connect (info_bar, "response",
                              G_CALLBACK (invalid_character_info_bar_response), tab);
        }
        else if (error->domain == GTK_SOURCE_FILE_SAVER_ERROR ||
                 (error->domain == G_IO_ERROR &&
                 error->code != G_IO_ERROR_INVALID_DATA &&
                 error->code != G_IO_ERROR_PARTIAL_INPUT))
        {
            /* These errors are _NOT_ recoverable */
            _xed_recent_remove (XED_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (tab))), location);

            info_bar = xed_unrecoverable_saving_error_info_bar_new (location, error);
            g_return_if_fail (info_bar != NULL);

            g_signal_connect (info_bar, "response",
                              G_CALLBACK (unrecoverable_saving_error_info_bar_response), tab);
        }
        else
        {
            const GtkSourceEncoding *encoding;

            /* This error is recoverable */
            g_return_if_fail (error->domain == G_CONVERT_ERROR || error->domain == G_IO_ERROR);

            encoding = gtk_source_file_saver_get_encoding (saver);

            info_bar = xed_conversion_error_while_saving_info_bar_new (location, encoding, error);
            g_return_if_fail (info_bar != NULL);

            g_signal_connect (info_bar, "response",
                              G_CALLBACK (recoverable_saving_error_info_bar_response), tab);
        }

        set_info_bar (tab, info_bar);
        gtk_info_bar_set_default_response (GTK_INFO_BAR (info_bar), GTK_RESPONSE_CANCEL);
        gtk_widget_show (info_bar);
    }
    else
    {
        gchar *mime = xed_document_get_mime_type (doc);

        _xed_recent_add (XED_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (tab))), location, mime);
        g_free (mime);

        if (tab->priv->print_preview != NULL)
        {
            xed_tab_set_state (tab, XED_TAB_STATE_SHOWING_PRINT_PREVIEW);
        }
        else
        {
            xed_tab_set_state (tab, XED_TAB_STATE_NORMAL);
        }

        tab->priv->ask_if_externally_modified = TRUE;

        g_signal_emit_by_name (doc, "saved");
        g_task_return_boolean (tab->priv->task_saver, TRUE);
    }

    if (error != NULL)
    {
        g_error_free (error);
    }
}

static void
save (XedTab *tab)
{
    XedDocument *doc;
    SaverData *data;

    g_return_if_fail (G_IS_TASK (tab->priv->task_saver));

    xed_tab_set_state (tab, XED_TAB_STATE_SAVING);

    doc = xed_tab_get_document (tab);
    g_signal_emit_by_name (doc, "save");

    data = g_task_get_task_data (tab->priv->task_saver);

    gtk_source_file_saver_save_async (data->saver,
                                      G_PRIORITY_DEFAULT,
                                      g_task_get_cancellable (tab->priv->task_saver),
                                      (GFileProgressCallback) saver_progress_cb,
                                      tab,
                                      NULL,
                                      (GAsyncReadyCallback) save_cb,
                                      tab);
}

/* Gets the initial save flags, when launching a new FileSaver. */
static GtkSourceFileSaverFlags
get_initial_save_flags (XedTab   *tab,
                        gboolean  auto_save)
{
    GtkSourceFileSaverFlags save_flags;
    gboolean create_backup;

    save_flags = tab->priv->save_flags;

    create_backup = g_settings_get_boolean (tab->priv->editor, XED_SETTINGS_CREATE_BACKUP_COPY);

    /* In case of autosaving, we need to preserve the backup that was produced
    * the last time the user "manually" saved the file. So we don't set the
    * CREATE_BACKUP flag for an automatic file saving.
    */
    if (create_backup && !auto_save)
    {
       save_flags |= GTK_SOURCE_FILE_SAVER_FLAGS_CREATE_BACKUP;
    }

    return save_flags;
}

void
_xed_tab_save_async (XedTab              *tab,
                     GCancellable        *cancellable,
                     GAsyncReadyCallback  callback,
                     gpointer             user_data)
{
    SaverData *data;
    XedDocument *doc;
    GtkSourceFile *file;
    GtkSourceFileSaverFlags save_flags;

    g_return_if_fail (XED_IS_TAB (tab));
    g_return_if_fail ((tab->priv->state == XED_TAB_STATE_NORMAL) ||
                      (tab->priv->state == XED_TAB_STATE_EXTERNALLY_MODIFIED_NOTIFICATION) ||
                      (tab->priv->state == XED_TAB_STATE_SHOWING_PRINT_PREVIEW));

    if (tab->priv->task_saver != NULL)
    {
        g_warning ("XedTab: file saver already exists.");
        return;
    }

    doc = xed_tab_get_document (tab);
    // g_return_if_fail (XED_IS_DOCUMENT (doc));
    g_return_if_fail (!xed_document_is_untitled (doc));

    tab->priv->task_saver = g_task_new (tab, cancellable, callback, user_data);

    data = saver_data_new ();
    g_task_set_task_data (tab->priv->task_saver,
                          data,
                          (GDestroyNotify) saver_data_free);

    save_flags = get_initial_save_flags (tab, FALSE);

    if (tab->priv->state == XED_TAB_STATE_EXTERNALLY_MODIFIED_NOTIFICATION)
    {
        /* We already told the user about the external modification:
         * hide the message bar and set the save flag.
         */

        set_info_bar (tab, NULL);
        save_flags |= GTK_SOURCE_FILE_SAVER_FLAGS_IGNORE_MODIFICATION_TIME;
    }

    file = xed_document_get_file (doc);

    data->saver = gtk_source_file_saver_new (GTK_SOURCE_BUFFER (doc), file);
    gtk_source_file_saver_set_flags (data->saver, save_flags);

    save (tab);
}

gboolean
_xed_tab_save_finish (XedTab       *tab,
                      GAsyncResult *result)
{
    gboolean success;

    g_return_val_if_fail (g_task_is_valid (result, tab), FALSE);
    g_return_val_if_fail (tab->priv->task_saver == G_TASK (result), FALSE);

    success = g_task_propagate_boolean (tab->priv->task_saver, NULL);
    g_clear_object (&tab->priv->task_saver);

    return success;
}

static void
auto_save_finished_cb (XedTab       *tab,
                       GAsyncResult *result,
                       gpointer      user_data)
{
    _xed_tab_save_finish (tab, result);
}

static gboolean
xed_tab_auto_save (XedTab *tab)
{
    SaverData *data;
    XedDocument *doc;
    GtkSourceFile *file;
    GtkSourceFileSaverFlags save_flags;

    xed_debug (DEBUG_TAB);

    doc = xed_tab_get_document (tab);
    g_return_val_if_fail (!xed_document_is_untitled (doc), G_SOURCE_REMOVE);
    g_return_val_if_fail (!xed_document_get_readonly (doc), G_SOURCE_REMOVE);

    if (!gtk_text_buffer_get_modified (GTK_TEXT_BUFFER (doc)))
    {
        xed_debug_message (DEBUG_TAB, "Document not modified");

        return G_SOURCE_CONTINUE;
    }

    if ((tab->priv->state != XED_TAB_STATE_NORMAL) && (tab->priv->state != XED_TAB_STATE_SHOWING_PRINT_PREVIEW))
    {
        xed_debug_message (DEBUG_TAB, "Retry after 30 seconds");

        tab->priv->auto_save_timeout = g_timeout_add_seconds (30, (GSourceFunc) xed_tab_auto_save, tab);

        /* Destroy the old timeout */
        return G_SOURCE_REMOVE;
    }

    /* Set auto_save_timeout to 0 since the timeout is going to be destroyed */
    tab->priv->auto_save_timeout = 0;

    if (tab->priv->task_saver != NULL)
    {
        g_warning ("XedTab: file saver already exists.");
        return G_SOURCE_REMOVE;
    }

    tab->priv->task_saver = g_task_new (tab,
                                        NULL,
                                        (GAsyncReadyCallback) auto_save_finished_cb,
                                        NULL);

    data = saver_data_new ();
    g_task_set_task_data (tab->priv->task_saver,
                          data,
                          (GDestroyNotify) saver_data_free);

    file = xed_document_get_file (doc);

    data->saver = gtk_source_file_saver_new (GTK_SOURCE_BUFFER (doc), file);

    save_flags = get_initial_save_flags (tab, TRUE);
    gtk_source_file_saver_set_flags (data->saver, save_flags);

    save (tab);

    return G_SOURCE_REMOVE;
}

/* Call _xed_tab_save_finish() in @callback, there is no
 * _xed_tab_save_as_finish().
 */
void
_xed_tab_save_as_async (XedTab                   *tab,
                        GFile                    *location,
                        const GtkSourceEncoding  *encoding,
                        GtkSourceNewlineType      newline_type,
                        GCancellable             *cancellable,
                        GAsyncReadyCallback       callback,
                        gpointer                  user_data)
{
    SaverData *data;
    XedDocument *doc;
    GtkSourceFile *file;
    GtkSourceFileSaverFlags save_flags;

    g_return_if_fail (XED_IS_TAB (tab));
    g_return_if_fail ((tab->priv->state == XED_TAB_STATE_NORMAL) ||
                      (tab->priv->state == XED_TAB_STATE_EXTERNALLY_MODIFIED_NOTIFICATION) ||
                      (tab->priv->state == XED_TAB_STATE_SHOWING_PRINT_PREVIEW));
    g_return_if_fail (G_IS_FILE (location));
    g_return_if_fail (encoding != NULL);

    if (tab->priv->task_saver != NULL)
    {
        g_warning ("XedTab: file saver already exists.");
        return;
    }

    tab->priv->task_saver = g_task_new (tab, cancellable, callback, user_data);

    data = saver_data_new ();
    g_task_set_task_data (tab->priv->task_saver,
                          data,
                          (GDestroyNotify) saver_data_free);

    doc = xed_tab_get_document (tab);
    g_return_if_fail (XED_IS_DOCUMENT (doc));

    /* reset the save flags, when saving as */
    tab->priv->save_flags = GTK_SOURCE_FILE_SAVER_FLAGS_NONE;
    save_flags = get_initial_save_flags (tab, FALSE);

    if (tab->priv->state == XED_TAB_STATE_EXTERNALLY_MODIFIED_NOTIFICATION)
    {
        /* We already told the user about the external modification:
         * hide the message bar and set the save flag.
         */

        set_info_bar (tab, NULL);
        save_flags |= GTK_SOURCE_FILE_SAVER_FLAGS_IGNORE_MODIFICATION_TIME;
    }

    file = xed_document_get_file (doc);

    data->saver = gtk_source_file_saver_new_with_target (GTK_SOURCE_BUFFER (doc), file, location);

    gtk_source_file_saver_set_encoding (data->saver, encoding);
    gtk_source_file_saver_set_newline_type (data->saver, newline_type);
    gtk_source_file_saver_set_flags (data->saver, save_flags);

    save (tab);
}

#define XED_PAGE_SETUP_KEY "xed-page-setup-key"
#define XED_PRINT_SETTINGS_KEY "xed-print-settings-key"

static GtkPageSetup *
get_page_setup (XedTab *tab)
{
    gpointer data;
    XedDocument *doc;

    doc = xed_tab_get_document (tab);
    data = g_object_get_data (G_OBJECT (doc), XED_PAGE_SETUP_KEY);

    if (data == NULL)
    {
        return _xed_app_get_default_page_setup (XED_APP (g_application_get_default ()));
    }
    else
    {
        return gtk_page_setup_copy (GTK_PAGE_SETUP (data));
    }
}

static GtkPrintSettings *
get_print_settings (XedTab *tab)
{
    gpointer data;
    XedDocument *doc;
    GtkPrintSettings *settings;
    gchar *uri, *name;

    doc = xed_tab_get_document (tab);

    data = g_object_get_data (G_OBJECT (doc), XED_PRINT_SETTINGS_KEY);

    if (data == NULL)
    {
        settings = _xed_app_get_default_print_settings (XED_APP (g_application_get_default ()));
    }
    else
    {
        settings = gtk_print_settings_copy (GTK_PRINT_SETTINGS (data));
    }

    name = xed_document_get_short_name_for_display (doc);
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
printing_cb (XedPrintJob       *job,
             XedPrintJobStatus  status,
             XedTab            *tab)
{
    g_return_if_fail (XED_IS_PROGRESS_INFO_BAR (tab->priv->info_bar));

    gtk_widget_show (tab->priv->info_bar);

    xed_progress_info_bar_set_text (XED_PROGRESS_INFO_BAR (tab->priv->info_bar),
                                    xed_print_job_get_status_string (job));

    xed_progress_info_bar_set_fraction (XED_PROGRESS_INFO_BAR (tab->priv->info_bar),
                                        xed_print_job_get_progress (job));
}

static void
store_print_settings (XedTab      *tab,
                      XedPrintJob *job)
{
    XedDocument *doc;
    GtkPrintSettings *settings;
    GtkPageSetup *page_setup;

    doc = xed_tab_get_document (tab);

    settings = xed_print_job_get_print_settings (job);

    /* clear n-copies settings since we do not want to
     * persist that one */
    gtk_print_settings_unset (settings, GTK_PRINT_SETTINGS_N_COPIES);

    /* remember settings for this document */
    g_object_set_data_full (G_OBJECT (doc), XED_PRINT_SETTINGS_KEY,
                            g_object_ref (settings), (GDestroyNotify)g_object_unref);

    /* make them the default */
    _xed_app_set_default_print_settings (XED_APP (g_application_get_default ()), settings);

    page_setup = xed_print_job_get_page_setup (job);

    /* remember page setup for this document */
    g_object_set_data_full (G_OBJECT (doc), XED_PAGE_SETUP_KEY,
                            g_object_ref (page_setup), (GDestroyNotify)g_object_unref);

    /* make it the default */
    _xed_app_set_default_page_setup (XED_APP (g_application_get_default ()), page_setup);
}

static void
done_printing_cb (XedPrintJob       *job,
                  XedPrintJobResult  result,
                  const GError      *error,
                  XedTab            *tab)
{
    XedView *view;

    g_return_if_fail (tab->priv->state == XED_TAB_STATE_PRINT_PREVIEWING ||
                      tab->priv->state == XED_TAB_STATE_SHOWING_PRINT_PREVIEW ||
                      tab->priv->state == XED_TAB_STATE_PRINTING);

    if (tab->priv->state == XED_TAB_STATE_SHOWING_PRINT_PREVIEW)
    {
        /* print preview has been destroyed... */
        tab->priv->print_preview = NULL;
    }
    else
    {
        g_return_if_fail (XED_IS_PROGRESS_INFO_BAR (tab->priv->info_bar));

        set_info_bar (tab, NULL); /* destroy the message area */
    }

    // TODO: check status and error

    if (result ==  XED_PRINT_JOB_RESULT_OK)
    {
        store_print_settings (tab, job);
    }

#if 0
    if (tab->priv->print_preview != NULL)
    {
        /* If we were printing while showing the print preview,
           see bug #352658 */
        gtk_widget_destroy (tab->priv->print_preview);
        g_return_if_fail (tab->priv->state == XED_TAB_STATE_PRINTING);
    }
#endif

    xed_tab_set_state (tab, XED_TAB_STATE_NORMAL);

    view = xed_tab_get_view (tab);
    gtk_widget_grab_focus (GTK_WIDGET (view));

    g_object_unref (tab->priv->print_job);
    tab->priv->print_job = NULL;
}

#if 0
static void
print_preview_destroyed (GtkWidget *preview,
             XedTab  *tab)
{
    tab->priv->print_preview = NULL;

    if (tab->priv->state == XED_TAB_STATE_SHOWING_PRINT_PREVIEW)
    {
        XedView *view;

        xed_tab_set_state (tab, XED_TAB_STATE_NORMAL);

        view = xed_tab_get_view (tab);
        gtk_widget_grab_focus (GTK_WIDGET (view));
    }
    else
    {
        /* This should happen only when printing while showing the print
         * preview. In this case let us continue whithout changing
         * the state and show the document. See bug #352658 */
        gtk_widget_show (tab->priv->view_scrolled_window);

        g_return_if_fail (tab->priv->state == XED_TAB_STATE_PRINTING);
    }
}
#endif

static void
show_preview_cb (XedPrintJob     *job,
                 XedPrintPreview *preview,
                 XedTab          *tab)
{
//  g_return_if_fail (tab->priv->state == XED_TAB_STATE_PRINT_PREVIEWING);
    g_return_if_fail (tab->priv->print_preview == NULL);

    set_info_bar (tab, NULL); /* destroy the message area */

    tab->priv->print_preview = GTK_WIDGET (preview);
    gtk_box_pack_end (GTK_BOX (tab), tab->priv->print_preview, TRUE, TRUE, 0);
    gtk_widget_show (tab->priv->print_preview);
    gtk_widget_grab_focus (tab->priv->print_preview);

/* when the preview gets destroyed we get "done" signal
    g_signal_connect (tab->priv->print_preview,
              "destroy",
              G_CALLBACK (print_preview_destroyed),
              tab);
*/
    xed_tab_set_state (tab, XED_TAB_STATE_SHOWING_PRINT_PREVIEW);
}

#if 0

static void
set_print_preview (XedTab  *tab,
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
preview_finished_cb (GtkSourcePrintJob *pjob, XedTab *tab)
{
    MatePrintJob *gjob;
    GtkWidget *preview = NULL;

    g_return_if_fail (XED_IS_PROGRESS_INFO_BAR (tab->priv->info_bar));
    set_info_bar (tab, NULL); /* destroy the message area */

    gjob = gtk_source_print_job_get_print_job (pjob);

    preview = xed_print_job_preview_new (gjob);
    g_object_unref (gjob);

    set_print_preview (tab, preview);

    gtk_widget_show (preview);
    g_object_unref (pjob);

    xed_tab_set_state (tab, XED_TAB_STATE_SHOWING_PRINT_PREVIEW);
}


#endif

static void
print_cancelled (GtkWidget *area,
                 gint       response_id,
                 XedTab    *tab)
{
    g_return_if_fail (XED_IS_PROGRESS_INFO_BAR (tab->priv->info_bar));

    xed_print_job_cancel (tab->priv->print_job);

    g_debug ("print_cancelled");
}

static void
show_printing_info_bar (XedTab   *tab,
                        gboolean  preview)
{
    GtkWidget *area;

    if (preview)
    {
        area = xed_progress_info_bar_new ("document-print-preview-symbolic", "", TRUE);
    }
    else
    {
        area = xed_progress_info_bar_new ("document-print-symbolic", "", TRUE);
    }

    g_signal_connect (area, "response",
                      G_CALLBACK (print_cancelled), tab);

    set_info_bar (tab, area);
}

static void
xed_tab_print_or_print_preview (XedTab                  *tab,
                                GtkPrintOperationAction  print_action)
{
    XedView *view;
    gboolean is_preview;
    GtkPageSetup *setup;
    GtkPrintSettings *settings;
    GtkPrintOperationResult res;
    GError *error = NULL;

    g_return_if_fail (tab->priv->print_job == NULL);
    g_return_if_fail (tab->priv->state == XED_TAB_STATE_NORMAL);

    view = xed_tab_get_view (tab);

    is_preview = (print_action == GTK_PRINT_OPERATION_ACTION_PREVIEW);

    tab->priv->print_job = xed_print_job_new (view);
    g_object_add_weak_pointer (G_OBJECT (tab->priv->print_job), (gpointer *) &tab->priv->print_job);

    show_printing_info_bar (tab, is_preview);

    g_signal_connect (tab->priv->print_job, "printing",
                      G_CALLBACK (printing_cb), tab);
    g_signal_connect (tab->priv->print_job, "show-preview",
                      G_CALLBACK (show_preview_cb), tab);
    g_signal_connect (tab->priv->print_job, "done",
                      G_CALLBACK (done_printing_cb), tab);

    if (is_preview)
    {
        xed_tab_set_state (tab, XED_TAB_STATE_PRINT_PREVIEWING);
    }
    else
    {
        xed_tab_set_state (tab, XED_TAB_STATE_PRINTING);
    }

    setup = get_page_setup (tab);
    settings = get_print_settings (tab);

    res = xed_print_job_print (tab->priv->print_job,
                               print_action,
                               setup,
                               settings,
                               GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (tab))),
                               &error);

    // TODO: manage res in the correct way
    if (res == GTK_PRINT_OPERATION_RESULT_ERROR)
    {
        /* FIXME: go in error state */
        xed_tab_set_state (tab, XED_TAB_STATE_NORMAL);
        g_warning ("Async print preview failed (%s)", error->message);
        g_object_unref (tab->priv->print_job);
        g_error_free (error);
    }
}

void
_xed_tab_print (XedTab *tab)
{
    g_return_if_fail (XED_IS_TAB (tab));

    /* FIXME: currently we can have just one printoperation going on
     * at a given time, so before starting the print we close the preview.
     * Would be nice to handle it properly though */
    if (tab->priv->state == XED_TAB_STATE_SHOWING_PRINT_PREVIEW)
    {
        gtk_widget_destroy (tab->priv->print_preview);
    }

    xed_tab_print_or_print_preview (tab, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG);
}

void
_xed_tab_print_preview (XedTab *tab)
{
    g_return_if_fail (XED_IS_TAB (tab));

    xed_tab_print_or_print_preview (tab, GTK_PRINT_OPERATION_ACTION_PREVIEW);
}

void
_xed_tab_mark_for_closing (XedTab *tab)
{
    g_return_if_fail (XED_IS_TAB (tab));
    g_return_if_fail (tab->priv->state == XED_TAB_STATE_NORMAL);

    xed_tab_set_state (tab, XED_TAB_STATE_CLOSING);
}

gboolean
_xed_tab_get_can_close (XedTab *tab)
{
    XedDocument *doc;
    XedTabState  ts;

    g_return_val_if_fail (XED_IS_TAB (tab), FALSE);

    ts = xed_tab_get_state (tab);

    /* if we are loading or reverting, the tab can be closed */
    if ((ts == XED_TAB_STATE_LOADING)       ||
        (ts == XED_TAB_STATE_LOADING_ERROR) ||
        (ts == XED_TAB_STATE_REVERTING)     ||
        (ts == XED_TAB_STATE_REVERTING_ERROR)) /* CHECK: I'm not sure this is the right behavior for REVERTING ERROR */
        {
            return TRUE;
        }

    /* Do not close tab with saving errors */
    if (ts == XED_TAB_STATE_SAVING_ERROR)
    {
        return FALSE;
    }

    doc = xed_tab_get_document (tab);

    if (_xed_document_needs_saving (doc))
    {
        return FALSE;
    }

    return TRUE;
}

/**
 * xed_tab_get_auto_save_enabled:
 * @tab: a #XedTab
 *
 * Gets the current state for the autosave feature
 *
 * Return value: %TRUE if the autosave is enabled, else %FALSE
 **/
gboolean
xed_tab_get_auto_save_enabled (XedTab *tab)
{
    xed_debug (DEBUG_TAB);

    g_return_val_if_fail (XED_IS_TAB (tab), FALSE);

    return tab->priv->auto_save;
}

/**
 * xed_tab_set_auto_save_enabled:
 * @tab: a #XedTab
 * @enable: enable (%TRUE) or disable (%FALSE) auto save
 *
 * Enables or disables the autosave feature. It does not install an
 * autosave timeout if the document is new or is read-only
 **/
void
xed_tab_set_auto_save_enabled (XedTab   *tab,
                               gboolean  enable)
{
    xed_debug (DEBUG_TAB);

    g_return_if_fail (XED_IS_TAB (tab));

    enable = enable != FALSE;

    if (tab->priv->auto_save != enable)
    {
        tab->priv->auto_save = enable;
        update_auto_save_timeout (tab);
        return;
    }
}

/**
 * xed_tab_get_auto_save_interval:
 * @tab: a #XedTab
 *
 * Gets the current interval for the autosaves
 *
 * Return value: the value of the autosave
 **/
gint
xed_tab_get_auto_save_interval (XedTab *tab)
{
    xed_debug (DEBUG_TAB);

    g_return_val_if_fail (XED_IS_TAB (tab), 0);

    return tab->priv->auto_save_interval;
}

/**
 * xed_tab_set_auto_save_interval:
 * @tab: a #XedTab
 * @interval: the new interval
 *
 * Sets the interval for the autosave feature.
 **/
void
xed_tab_set_auto_save_interval (XedTab *tab,
                                gint    interval)
{
    g_return_if_fail (XED_IS_TAB (tab));
    g_return_if_fail (interval > 0);

    xed_debug (DEBUG_TAB);

    if (tab->priv->auto_save_interval != interval)
    {
        tab->priv->auto_save_interval = interval;
        remove_auto_save_timeout (tab);
        update_auto_save_timeout (tab);
    }
}

void
xed_tab_set_info_bar (XedTab    *tab,
                      GtkWidget *info_bar)
{
    g_return_if_fail (XED_IS_TAB (tab));
    g_return_if_fail (info_bar == NULL || GTK_IS_WIDGET (info_bar));

    /* FIXME: this can cause problems with the tab state machine */
    set_info_bar (tab, info_bar);
}

GtkWidget *
_xed_tab_get_view_frame (XedTab *tab)
{
    return GTK_WIDGET (tab->priv->frame);
}
